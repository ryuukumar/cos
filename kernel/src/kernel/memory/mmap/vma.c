/*
 * vma.c
 * Copyright (C) 2026  Aditya Kumar
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, see <https://www.gnu.org/licenses/>.
 */

#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/memmgt.h>
#include <kernel/memory/vma.h>
#include <liballoc/liballoc.h>
#include <utils/rbtree.h>
#include <utils/spinlock.h>

static int vma_comp (const rbtree_elem* a, const rbtree_elem* b) {
	const vma_alloc *a_vma = (const vma_alloc*)*a, *b_vma = (const vma_alloc*)*b;
	if (a_vma->mem_start > b_vma->mem_start)
		return 1;
	else if (a_vma->mem_start < b_vma->mem_start)
		return -1;
	return 0;
}

static void vma_freer (const rbtree_elem* e) {
	const vma_alloc* v = (const vma_alloc*)*e;
	kfree ((void*)v);
}

vma* create_vma () {
	vma* new_vma = kmalloc (sizeof (vma));
	if (!new_vma) return nullptr;

	kmemset (new_vma, 0, sizeof (vma));

	new_vma->vma_rbtree = rbtree_create (vma_comp);
	if (!new_vma->vma_rbtree) {
		kfree (new_vma);
		return nullptr;
	}

	rbtree_set_freer (new_vma->vma_rbtree, vma_freer);

	new_vma->min_alloc = vma_min_alloc;
	new_vma->max_alloc = vma_max_alloc;

	return new_vma;
}

void destroy_vma (vma* vmaobj) {
	if (!vmaobj) return;

	uint64_t flags = spinlock_acquire (&vmaobj->lock);

	rbtree_destroy (vmaobj->vma_rbtree);
	vmaobj->max_alloc = vmaobj->min_alloc = 0;

	spinlock_release (&vmaobj->lock, flags);
	kfree (vmaobj);
}

static int64_t __vma_dealloc_block (vma* vmaobj, uint64_t mem_start, uint64_t mem_len,
									uint64_t cr3) {
	uint64_t mem_end = mem_start + mem_len;
	int64_t	 error = 0;

	while (true) {
		vma_alloc  comp = {.mem_start = mem_end};
		vma_alloc* res = nullptr;

		error = rbtree_find_atmost (vmaobj->vma_rbtree, (rbtree_elem)&comp, (rbtree_elem*)&res);
		if (error != 0 && error != -INTERNAL_ENOTFOUND) return error;

		// case 0: nothing before our range
		if (error == -INTERNAL_ENOTFOUND) {
			error = 0;
			break;
		}

		uint64_t res_start = res->mem_start, res_end_incl = res->mem_start + res->mem_len - 1,
				 arg_start = mem_start, arg_end_incl = mem_start + mem_len - 1;

		// for all following cases: {} is detected vma_alloc and [] is given range

		// case 1: {..}..[..]
		if (res_end_incl < arg_start) break;

		// case 2: {..[..}..]
		else if (res_start < arg_start && arg_start <= res_end_incl &&
				 res_end_incl <= arg_end_incl) {
			uint64_t new_len = arg_start - res_start;
			dealloc_by_cr3 (cr3, arg_start, (res_end_incl - arg_start + 1) / PAGE_SIZE);
			res->mem_len = new_len;
		}

		// case 3: [..{..}..]
		else if (arg_start <= res_start && res_end_incl <= arg_end_incl) {
			dealloc_by_cr3 (cr3, res_start, res->mem_len / PAGE_SIZE);
			error = rbtree_delete (vmaobj->vma_rbtree, (rbtree_elem)res);
			if (error) return error;
		}

		// case 4: [..{..]..}
		else if (arg_start <= res_start && res_start <= arg_end_incl &&
				 arg_end_incl < res_end_incl) {
			uint64_t new_start = arg_end_incl + 1, new_len = res_end_incl - new_start + 1;
			dealloc_by_cr3 (cr3, res_start, (res->mem_len - new_len) / PAGE_SIZE);
			res->mem_start = new_start;
			res->mem_len = new_len;
			mem_end--; // prevent infinite loop
		}

		// case 5: {..[..]..}
		else if (res_start < arg_start && arg_end_incl < res_end_incl) {
			vma_alloc* new_b2 = kmalloc (sizeof (vma_alloc));
			if (!new_b2) return -ENOMEM;
			kmemcpy (new_b2, res, sizeof (vma_alloc));
			new_b2->mem_start = arg_end_incl + 1;
			new_b2->mem_len = res_end_incl - new_b2->mem_start + 1;

			error = rbtree_insert (vmaobj->vma_rbtree, (rbtree_elem)new_b2);
			if (error) {
				kfree (new_b2);
				return error;
			}

			dealloc_by_cr3 (cr3, arg_start, (mem_len) / PAGE_SIZE);
			res->mem_len = arg_start - res_start;

			break; // no other intersecting vma_alloc objects exist
		}

		else
			break;
	}

	return 0;
}

static int64_t __vma_alloc_block (vma* vmaobj, uint64_t mem_start, uint64_t mem_len, uint64_t cr3,
								  uint8_t flags, void* backing, size_t offset) {
	uint64_t mem_end = mem_start + mem_len;
	int64_t	 error = 0;

	if (flags & MEM_NOW) {
		vma_alloc  comp = {.mem_start = mem_end - 1};
		vma_alloc* res = nullptr;

		error = rbtree_find_atmost (vmaobj->vma_rbtree, (rbtree_elem)&comp, (rbtree_elem*)&res);
		if (error != 0 && error != -INTERNAL_ENOTFOUND) return error;

		if (!(error == -INTERNAL_ENOTFOUND || res->mem_start + res->mem_len - 1 < mem_start))
			return -EEXIST;
	}

	__vma_dealloc_block (vmaobj, mem_start, mem_len, cr3);

	// check if we can extend a predecessor
	vma_alloc  tgt_comp = {.mem_start = mem_start};
	vma_alloc* tgt_alloc = nullptr;

	error =
		rbtree_find_atmost (vmaobj->vma_rbtree, (rbtree_elem)&tgt_comp, (rbtree_elem*)&tgt_alloc);
	if (error != 0 && error != -INTERNAL_ENOTFOUND) return error;

	if (tgt_alloc && tgt_alloc->mem_start + tgt_alloc->mem_len == mem_start &&
		tgt_alloc->mem_flags == (flags & (~MEM_NOW)) && tgt_alloc->mem_backing == backing &&
		tgt_alloc->f_offset + tgt_alloc->mem_len == offset) {
		tgt_alloc->mem_len += mem_len;
	} else {
		tgt_alloc = kmalloc (sizeof (vma_alloc));
		if (!tgt_alloc) return -ENOMEM;

		tgt_alloc->mem_start = mem_start;
		tgt_alloc->mem_len = mem_len;
		tgt_alloc->mem_flags = flags & (~MEM_NOW);

		error = rbtree_insert (vmaobj->vma_rbtree, (rbtree_elem)tgt_alloc);
		if (error) {
			kfree (tgt_alloc);
			return error;
		}
	}

	// check if we can extend to cover a successor
	vma_alloc  succ_comp = {.mem_start = mem_start + mem_len};
	vma_alloc* succ_res = nullptr;

	error = rbtree_find (vmaobj->vma_rbtree, (rbtree_elem)&succ_comp, (rbtree_elem*)&succ_res);
	if (error != 0 && error != -INTERNAL_ENOTFOUND) return error;

	if (succ_res && succ_res->mem_flags == (flags & (~MEM_NOW)) &&
		tgt_alloc->mem_backing == succ_res->mem_backing &&
		tgt_alloc->f_offset + tgt_alloc->mem_len == succ_res->f_offset) {
		tgt_alloc->mem_len += succ_res->mem_len;
		error = rbtree_delete (vmaobj->vma_rbtree, (rbtree_elem)succ_res);
		if (error) return error;
	}

	// allocate the actual memory needed
	alloc_by_cr3 (cr3, mem_start, mem_len / PAGE_SIZE,
				  M_PG_READ | ((flags & MEM_W) ? M_PG_WRITE : 0) |
					  ((flags & MEM_X) ? M_PG_EXEC : 0));

	error = 0;
	return error;
}

int64_t vma_alloc_block (vma* vmaobj, uint64_t mem_start, uint64_t mem_len, uint64_t cr3,
						 uint8_t flags, void* backing, size_t offset) {
	if (!vmaobj || !vmaobj->vma_rbtree) return -EINVAL;
	if (mem_start == 0 || mem_len == 0) return -EINVAL;
	if (mem_start % PAGE_SIZE || mem_len % PAGE_SIZE) return -INTERNAL_EBADADDR;

	uint64_t slflags = spinlock_acquire (&vmaobj->lock);
	int64_t	 error = __vma_alloc_block (vmaobj, mem_start, mem_len, cr3, flags, backing, offset);
	spinlock_release (&vmaobj->lock, slflags);
	return error;
}

int64_t vma_dealloc_block (vma* vmaobj, uint64_t mem_start, uint64_t mem_len, uint64_t cr3) {
	if (!vmaobj || !vmaobj->vma_rbtree) return -EINVAL;
	if (mem_start == 0 || mem_len == 0) return -EINVAL;
	if (mem_start % PAGE_SIZE || mem_len % PAGE_SIZE) return -INTERNAL_EBADADDR;

	uint64_t slflags = spinlock_acquire (&vmaobj->lock);
	int		 error = __vma_dealloc_block (vmaobj, mem_start, mem_len, cr3);
	spinlock_release (&vmaobj->lock, slflags);
	return error;
}

/*!
 * Retrieve the vma_alloc object which contains the passed address. The returned vma_alloc object is
 * guaranteed to be valid until the next mutation to the vma object (typically deletion). If there
 * is no vma object corresponding to this address, or the vma object is invalid, nullptr is
 * returned.
 *
 * The returned object, if valid, is considered read-only, and changing paramters would cause
 * undefined behavior.
 *
 * @param vmaobj pointer to VMA object
 * @param address address to search for
 * @return pointer to vma_alloc object if found, else nullptr
 */
const vma_alloc* get_vma_alloc_by_addr (vma* vmaobj, uint64_t address) {
	if (!vmaobj || !vmaobj->vma_rbtree) return nullptr;

	vma_alloc  comp = {.mem_start = address};
	vma_alloc* res = nullptr;

	uint64_t flags = spinlock_acquire (&vmaobj->lock);
	int error = rbtree_find_atmost (vmaobj->vma_rbtree, (rbtree_elem)&comp, (rbtree_elem*)&res);
	spinlock_release (&vmaobj->lock, flags);
	if (error) return nullptr;

	return (address >= res->mem_start && address < res->mem_start + res->mem_len) ? res : nullptr;
}