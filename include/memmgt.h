#ifndef MEMMGT_H
#define MEMMGT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <limine.h>

typedef struct {
	uint64_t present : 1;               // Page present in memory
	uint64_t read_write : 1;            // Read-write flag
	uint64_t user_supervisor : 1;       // User-supervisor flag
	uint64_t page_write_through : 1;    // Page-level write-through
	uint64_t page_cache_disable : 1;    // Page-level cache disable
	uint64_t accessed : 1;              // Accessed flag
	uint64_t ignored1 : 1;              // Ignored
	uint64_t page_size : 1;             // Page size (must be 0 in PML4)
	uint64_t ignored2 : 4;              // Ignored
	uint64_t pdpt_base_address : 40;    // PDPT base address (physical)
	uint64_t ignored3 : 11;             // Ignored
	uint64_t xd : 1;                    // Execute-disable bit
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
	uint64_t present    : 1;
	uint64_t rw         : 1;
	uint64_t us         : 1;
	uint64_t pwt        : 1;
	uint64_t pcd        : 1;
	uint64_t accessed   : 1;
	uint64_t ignored1   : 1;
	uint64_t page_size  : 1;
	uint64_t ignored2   : 1;
	uint64_t available1 : 3;
	uint64_t pt_base_address : 40;
	uint64_t available2 : 11;
	uint64_t nex        : 1;
} pd_entry_t;

typedef struct {
	uint64_t present    : 1;
	uint64_t rw         : 1;
	uint64_t us         : 1;
	uint64_t pwt        : 1;
	uint64_t pcd        : 1;
	uint64_t accessed   : 1;
	uint64_t dirty      : 1;
	uint64_t pat        : 1;
	uint64_t global     : 1;
	uint64_t allocated  : 1;
	uint64_t available1 : 2;
	uint64_t frame_base_address : 40;
	uint64_t available2 : 11;
	uint64_t nex        : 1;
} pt_entry_t;

void init_memmgt(uint64_t, struct limine_memmap_response*);
void walk_pagetable(void);
void* get_paddr(void* vaddr);

uint64_t allocate_physical_pageframes(size_t);

#endif