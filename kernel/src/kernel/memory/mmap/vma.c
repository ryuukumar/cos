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
