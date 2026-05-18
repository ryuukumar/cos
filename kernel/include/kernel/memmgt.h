/*
 * memmgt.h
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

#include <kernel/limine.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define USER_PML4_IDX 1
#define KRNL_PML4_IDX 257

#define PAGE_SIZE		   4096ull
#define ALIGN_PAGE_DOWN(x) ((x) & ~(PAGE_SIZE - 1))

#define M_PG_READ  0x0001
#define M_PG_WRITE 0x0002
#define M_PG_EXEC  0x0004
#define M_PG_USER  0x0008

#define M_PG_IS_READ(x)	 ((x) & M_PG_READ)
#define M_PG_IS_WRITE(x) ((x) & M_PG_WRITE)
#define M_PG_IS_EXEC(x)	 ((x) & M_PG_EXEC)
#define M_PG_IS_USER(x)	 ((x) & M_PG_USER)

typedef struct {
	uint64_t present : 1;			 // Page present in memory
	uint64_t read_write : 1;		 // Read-write flag
	uint64_t user_supervisor : 1;	 // User-supervisor flag
	uint64_t page_write_through : 1; // Page-level write-through
	uint64_t page_cache_disable : 1; // Page-level cache disable
	uint64_t accessed : 1;			 // Accessed flag
	uint64_t ignored1 : 1;			 // Ignored
	uint64_t page_size : 1;			 // Page size (must be 0 in PML4)
	uint64_t ignored2 : 4;			 // Ignored
	uint64_t pdpt_base_address : 40; // PDPT base address (physical)
	uint64_t ignored3 : 11;			 // Ignored
	uint64_t xd : 1;				 // Execute-disable bit
} pml4t_entry_t;

typedef struct {
	uint64_t present : 1;
	uint64_t read_write : 1;
	uint64_t user_supervisor : 1;
	uint64_t page_write_through : 1;
	uint64_t page_cache_disable : 1;
	uint64_t accessed : 1;
	uint64_t ignored1 : 1;
	uint64_t page_size : 1;
	uint64_t ignored2 : 4;
	uint64_t pd_base_address : 40;
	uint64_t ignored3 : 11;
	uint64_t xd : 1;
} pdpt_entry_t;

typedef struct {
	uint64_t present : 1;
	uint64_t rw : 1;
	uint64_t us : 1;
	uint64_t pwt : 1;
	uint64_t pcd : 1;
	uint64_t accessed : 1;
	uint64_t ignored1 : 1;
	uint64_t page_size : 1;
	uint64_t ignored2 : 1;
	uint64_t available1 : 3;
	uint64_t pt_base_address : 40;
	uint64_t available2 : 11;
	uint64_t nex : 1;
} pd_entry_t;

typedef struct {
	uint64_t present : 1;
	uint64_t rw : 1;
	uint64_t us : 1;
	uint64_t pwt : 1;
	uint64_t pcd : 1;
	uint64_t accessed : 1;
	uint64_t dirty : 1;
	uint64_t pat : 1;
	uint64_t global : 1;
	uint64_t available1 : 3;
	uint64_t frame_base_address : 40;
	uint64_t available2 : 11;
	uint64_t nex : 1;
} pt_entry_t;

typedef struct {
	uint64_t pages_base;
	uint64_t pages_maxlen;
	uint64_t pages_used;
	uint8_t* map;
} memmap_bitmap;

typedef struct {
	uint16_t pml4_index;
	uint16_t pdpt_index;
	uint16_t pd_index;
	uint16_t pt_index;
	uint16_t offset;
} vaddr_t;

typedef uint64_t* paddr_t;

uint64_t read_cr3 (void);
void	 write_cr3 (uint64_t new_value);

vaddr_t get_vaddr_t_from_ptr (void* ptr);
void*	get_vaddr_from_frame (uint64_t phys_frame);
void*	get_vaddr_from_phys_addr (uint64_t phys_address);
void*	vaddr_t_to_ptr (vaddr_t* virtual_addr);
bool	is_vaddr_t_user (vaddr_t* addr);
bool	is_vaddr_t_lt (vaddr_t* a, vaddr_t* b);
bool	is_vaddr_t_eq (vaddr_t a, vaddr_t b);

void* alloc_vpages (size_t req_count, uint16_t flags);
void* alloc_vpage (uint16_t flags);
void  free_vpages (void* ptr, size_t count);
void  free_vpage (void* ptr);
void  init_vmm (pml4t_entry_t* kernel_pml4);

void alloc_all_vpages_in_range (vaddr_t first, vaddr_t last, paddr_t base_addr, uint16_t flags);
void free_all_vpages_in_range (vaddr_t first, vaddr_t last);
void reflag_all_vpages_in_range (vaddr_t first, vaddr_t last, uint16_t flags);
void alloc_by_cr3 (uint64_t cr3, uintptr_t start, size_t num_pages, uint16_t flags);
void dealloc_by_cr3 (uint64_t cr3, uintptr_t start, size_t num_pages);
void reflag_by_cr3 (uint64_t cr3, uintptr_t start, size_t num_pages, uint16_t flags);

void  init_memmgt (uint64_t, struct limine_memmap_response*);
void  walk_pagetable (void);
void* get_paddr (void* vaddr);

uintptr_t	   get_kernel_cr3 (void);
uint64_t	   get_hhdm_offset (void);
pml4t_entry_t* get_pml4_baseptr (void);

int clone_user_memory (uint64_t cr3_src, uint64_t* cr3_dest);

// LIBALLOC FUNCTION IMPLEMENTATIONS

void* try_assign_pt (pt_entry_t*, size_t);
int	  liballoc_lock (void);
int	  liballoc_unlock (void);
void* liballoc_alloc (size_t);
int	  liballoc_free (void*, size_t);
