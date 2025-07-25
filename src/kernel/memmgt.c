#include <kernel/limine.h>
#include <kernel/memmgt.h>
#include <memory.h>
#include <stdio.h>






#define PAGE_SIZE 4096

uint64_t alloc_frames_base  = 0;
uint64_t alloc_frames_count = 0;
uint64_t alloc_frames_limit = 0;

pml4t_entry_t* pml4_base_ptr = NULL;
uint64_t hhdm_offset = 0;

bool is_locked = false;

extern struct {
	pdpt_entry_t pdpt_entry[512];
	pd_entry_t   pd_entry[512];
	pt_entry_t   pt_entry[512];
} memmap;


/*!
 * Reads the value of the CR3 register, which contains the physical address of the PML4 table.
 * @return The value of the CR3 register.
 */
uint64_t read_cr3() {
	uint64_t cr3;
	__asm__ volatile ("mov %%cr3, %0" : "=r" (cr3));
	return cr3;
}

/*!
 * Initializes the memory management subsystem.
 * Sets the base pointer for the PML4 table and stores the HHDM offset.
 * @param p_hhdm_offset The higher half direct mapping offset.
 */
void init_memmgt(uint64_t p_hhdm_offset, struct limine_memmap_response* memmap_response) {
	uint64_t pml4_base = read_cr3() & 0xFFFFFFFFFF000;
	pml4_base_ptr = (pml4t_entry_t*)(pml4_base + p_hhdm_offset);
	hhdm_offset = p_hhdm_offset;

	uint64_t highest_limit_so_far = 0;
	uint64_t best_base_so_far = 0xffffffffffffffff;

	// set permissible limit
	if (memmap_response != NULL) {
		for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
			if (memmap_response->entries[i]->type == 0) {
				if (memmap_response->entries[i]->length > highest_limit_so_far) {
					highest_limit_so_far = memmap_response->entries[i]->length;
					best_base_so_far = memmap_response->entries[i]->base;
				}
			}
		}
	}

	if (highest_limit_so_far == 0) {
		printf("It's so joever for physical memory\n");
		return;
	}

	alloc_frames_base  = best_base_so_far;
	alloc_frames_limit = highest_limit_so_far + best_base_so_far;

	// set up default memory map
	memset(&memmap, 0, sizeof(memmap));

	for (int i = 0; i < 1; i++) {
		memmap.pdpt_entry[i].present = 1;
		memmap.pdpt_entry[i].read_write = 1;
		memmap.pdpt_entry[i].pd_base_address = ((uint64_t)(get_paddr(&memmap.pd_entry[i * 512])) >> 12) & 0xFFFFFFFFFF;
		memmap.pdpt_entry[i].xd = 1;
	}

	for (int i = 0; i < 1; i++) {
		memmap.pd_entry[i].present = 1;
		memmap.pd_entry[i].rw = 1;
		memmap.pd_entry[i].nex = 1;
		memmap.pd_entry[i].pt_base_address = ((uint64_t)(get_paddr(&memmap.pt_entry[i * 512])) >> 12) & 0xFFFFFFFFFF;
	}

	for (int i = 0; i < 512; i++) {
		memmap.pt_entry[i].present = 1;
		memmap.pt_entry[i].rw = 1;
		memmap.pt_entry[i].frame_base_address = ((uint64_t)(alloc_frames_base + PAGE_SIZE * i) >> 12) & 0xFFFFFFFFFF;
	}

	// initialise PML4T idx 1 and PDPT idx 0 for our page assignments
	uint64_t pdpt_base_address = ((uint64_t)(get_paddr(&memmap.pdpt_entry[0])));
	pml4t_entry_t pml4t_entry = {
		.present = 1,
		.read_write = 1,
		.user_supervisor = 0,
		.page_write_through = 0,
		.page_cache_disable = 0,
		.accessed = 0,
		.ignored1 = 0,
		.page_size = 0, // must be 0 in PML4
		.ignored2 = 0,
		.pdpt_base_address = pdpt_base_address >> 12 & 0xFFFFFFFFFF, // physical address of PDPT
		.ignored3 = 0,
		.xd = 1 // execute-disable bit
	};
	pml4_base_ptr[1] = pml4t_entry;
}

uint64_t allocate_physical_pageframes(size_t count) {
	if (alloc_frames_base + (PAGE_SIZE * (alloc_frames_count + count)) > alloc_frames_limit) return 0;
	if (is_locked) return 0;

	uint64_t allocated_memory = alloc_frames_base + (PAGE_SIZE * alloc_frames_count);
	alloc_frames_count += count;
	return allocated_memory;
}

/*!
 * Walks the page table hierarchy and prints present entries and their address ranges.
 */
void walk_pagetable() {
	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[1];
	if (!pml4t_entry->present) return;

	pdpt_entry_t* pdpt_base_ptr = (pdpt_entry_t*)((pml4t_entry->pdpt_base_address << 12) + hhdm_offset);
	pdpt_entry_t* pdpt_entry = &pdpt_base_ptr[0];
	if (!pdpt_entry->present) return;

	pd_entry_t* pd_base_ptr = (pd_entry_t*)((pdpt_entry->pd_base_address << 12) + hhdm_offset);
	for (int k = 0; k < 512; k++) {
		pd_entry_t* pd_entry = &pd_base_ptr[k];
		if (!pd_entry->present) continue;
		printf("PD %d: PT Base Address:   0x%lx\n",
			k, pd_entry->pt_base_address << 12);
		pt_entry_t* pt_base_ptr = (pt_entry_t*)((pd_entry->pt_base_address << 12) + hhdm_offset);
				
		bool is_present_pt[512] = { false };
		for (int k = 0; k < 512; k++) is_present_pt[k] = (&pt_base_ptr[k])->present;

		// print ranges of present pd's
		int range_start = -1;
		for (int k = 0; k <= 512; k++) {
			if (k < 512 && is_present_pt[k]) {
				if (range_start == -1)
					range_start = k;
			}
			else {
				if (range_start != -1) {
					if (range_start == k - 1) {
						printf("  Present PT: %d\n", range_start);
					}
					else {
						printf("  Present PTs: %d-%d\n", range_start, k - 1);
					}
					range_start = -1;
				}
			}
		}
	}
}

/*!
 * Finds the physical address mapped to a given virtual address.
 * @param vaddr The virtual address to look up.
 * @return The corresponding physical address, or NULL if not mapped.
 */
void* get_paddr(void* vaddr) {
	uint64_t virtual_addr = (uint64_t)vaddr;

	uint64_t pml4_index = (virtual_addr >> 39) & 0x1FF;
	uint64_t pdpt_index = (virtual_addr >> 30) & 0x1FF;
	uint64_t pd_index = (virtual_addr >> 21) & 0x1FF;
	uint64_t pt_index = (virtual_addr >> 12) & 0x1FF;
	uint64_t page_offset = virtual_addr & 0xFFF;

	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[pml4_index];
	if (!pml4t_entry->present) return NULL;

	pdpt_entry_t* pdpt_base_ptr = (pdpt_entry_t*)((pml4t_entry->pdpt_base_address << 12) + hhdm_offset);
	pdpt_entry_t* pdpt_entry = &pdpt_base_ptr[pdpt_index];
	if (!pdpt_entry->present) return NULL;

	pd_entry_t* pd_base_ptr = (pd_entry_t*)((pdpt_entry->pd_base_address << 12) + hhdm_offset);
	pd_entry_t* pd_entry = &pd_base_ptr[pd_index];
	if (!pd_entry->present) return NULL;

	pt_entry_t* pt_base_ptr = (pt_entry_t*)((pd_entry->pt_base_address << 12) + hhdm_offset);
	pt_entry_t* pt_entry = &pt_base_ptr[pt_index];
	if (!pt_entry->present) return NULL;

	uint64_t phys_addr = (pt_entry->frame_base_address << 12) | page_offset;

	return (void*)phys_addr;
}





/*

LIBALLOC FUNCTION IMPLEMENTATIONS

*/

void* try_assign_pt(pt_entry_t* pt_base_ptr, size_t count) {
	int free_count = 0;
	for (int i = 0; i < 512; i++) {
		if (pt_base_ptr[i].allocated) {
			free_count = 0;
			continue;
		}
		else {
			free_count++;
			if ((size_t)free_count == count) {
				int init_idx = i - count + 1;

				for (int j = init_idx; j <= i; j++) {
					pt_base_ptr[j].allocated = 1;
				}

				void* alloc_addr = (void*)((1ll << 39) | ((uint64_t)init_idx << 12));
				return alloc_addr;
			}
		}
	}
	return NULL;
}

int liballoc_lock() {
	is_locked = true;
	return 0;
}

int liballoc_unlock() {
	is_locked = false;
	return 0;
}

void* liballoc_alloc(size_t count) {
	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[1];
	if (!pml4t_entry->present) return NULL;
	pdpt_entry_t* pdpt_entry = &((pdpt_entry_t*)((pml4t_entry->pdpt_base_address << 12) + hhdm_offset))[0];
	if (!pdpt_entry->present) return NULL;
	pd_entry_t* pd_entry = &((pd_entry_t*)((pdpt_entry->pd_base_address << 12) + hhdm_offset))[0];
	if (!pd_entry->present) return NULL;
	pt_entry_t* pt_base_ptr = (pt_entry_t*)((pd_entry->pt_base_address << 12) + hhdm_offset);

	return try_assign_pt(pt_base_ptr, count);
}

int liballoc_free(void* ptr, size_t count) {
	if (is_locked) return -7;
	if (get_paddr(ptr) == NULL) return -1;

	uint64_t virtual_addr = (uint64_t)ptr;

	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[(virtual_addr >> 39) & 0x1FF];
	if (!pml4t_entry->present || ((virtual_addr >> 39) & 0x1FF) != 1) return -2;
	pdpt_entry_t* pdpt_entry = &((pdpt_entry_t*)((pml4t_entry->pdpt_base_address << 12) + hhdm_offset))[(virtual_addr >> 30) & 0x1FF];
	if (!pdpt_entry->present || ((virtual_addr >> 30) & 0x1FF) != 0) return -3;
	pd_entry_t* pd_entry = &((pd_entry_t*)((pdpt_entry->pd_base_address << 12) + hhdm_offset))[(virtual_addr >> 21) & 0x1FF];
	if (!pd_entry->present || ((virtual_addr >> 21) & 0x1FF) != 0) return -4;
	pt_entry_t* pt_base_ptr = (pt_entry_t*)((pd_entry->pt_base_address << 12) + hhdm_offset);

	for (size_t i = 0; i < count; i++) {
		if (i + ((virtual_addr >> 12) & 0x1FF) >= 512) return -5; // out of bounds

		pt_entry_t* pt_entry = &pt_base_ptr[i + ((virtual_addr >> 12) & 0x1FF)];
		if (!pt_entry->allocated) return -6;

		pt_entry->allocated = 0;
	}

	return 0;
}