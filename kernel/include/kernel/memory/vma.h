/*
 * vma.h
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

#pragma once

#include <utils/rbtree.h>
#include <utils/spinlock.h>

static constexpr uint64_t vma_min_alloc = 0x01000000000000;
static constexpr uint64_t vma_max_alloc = 0x10000000000000;

#define MEM_R	0x00 // read enabled allocation
#define MEM_W	0x01 // write enabled allocation
#define MEM_X	0x02 // execute enabled allocation
#define MEM_NOW 0x04 // fail if overwriting previous allocations

typedef struct {
	uint64_t mem_start, mem_len;
	uint8_t	 mem_flags;
	void*	 mem_backing;
	uint64_t f_offset;
} vma_alloc;

typedef struct {
	rbtree*	 vma_rbtree;
	uint64_t min_alloc, max_alloc;
	bool	 lock;
} vma;

vma* create_vma ();
void destroy_vma (vma* vmaobj);

int64_t vma_alloc_block (vma* vmaobj, uint64_t mem_start, uint64_t mem_len, uint64_t cr3,
						 uint8_t flags, void* backing, size_t offset);
int64_t vma_reflag_block (vma* vmaobj, uint64_t mem_start, uint64_t mem_len, uint64_t cr3,
						  uint8_t flags);
int64_t vma_dealloc_block (vma* vmaobj, uint64_t mem_start, uint64_t mem_len, uint64_t cr3);

const vma_alloc* get_vma_alloc_by_addr (vma* vmaobj, uint64_t address);
