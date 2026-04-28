#include "kclib/stdio.h"
#include <kclib/string.h>
#include <kernel/memmgt.h>
#include <kernel/memory/pmm.h>

static pml4t_entry_t* pml4_base_ptr = nullptr;

/*!
 * Allocate all virtual pages in given range (inclusive). Needs physical memory to be allocated.
 * Potential bug when first and last are part of different PML4 entries.
 * @param first first virtual address in range
 * @param last last virtual address in range
 * @param base_addr base address of physical memory of corresponding size
 */
void alloc_all_vpages_in_range (vaddr_t first, vaddr_t last, paddr_t base_addr) {
	uint64_t phys_base_track = (uint64_t)base_addr;
	vaddr_t	 current = first;

	while (true) {
		pml4t_entry_t* pml4t_entry = &pml4_base_ptr[current.pml4_index];

		if (!pml4t_entry->present) {
			paddr_t new_table = alloc_ppage ();
			pml4t_entry->present = 1;
			pml4t_entry->read_write = 1;
			pml4t_entry->user_supervisor = is_vaddr_t_user (&current);
			pml4t_entry->pdpt_base_address = (uint64_t)new_table / PAGE_SIZE;
			kmemset (get_vaddr_from_frame ((uint64_t)new_table / PAGE_SIZE), 0, PAGE_SIZE);
		}

		pdpt_entry_t* pdpt_base =
			(pdpt_entry_t*)get_vaddr_from_frame (pml4t_entry->pdpt_base_address);
		pdpt_entry_t* pdpt_entry = &pdpt_base[current.pdpt_index];

		if (!pdpt_entry->present) {
			paddr_t new_table = alloc_ppage ();
			pdpt_entry->present = 1;
			pdpt_entry->read_write = 1;
			pdpt_entry->user_supervisor = is_vaddr_t_user (&current);
			pdpt_entry->pd_base_address = (uint64_t)new_table / PAGE_SIZE;
			kmemset (get_vaddr_from_frame ((uint64_t)new_table / PAGE_SIZE), 0, PAGE_SIZE);
		}

		pd_entry_t* pd_base = (pd_entry_t*)get_vaddr_from_frame (pdpt_entry->pd_base_address);
		pd_entry_t* pd_entry = &pd_base[current.pd_index];

		if (!pd_entry->present) {
			paddr_t new_table = alloc_ppage ();
			pd_entry->present = 1;
			pd_entry->rw = 1;
			pd_entry->us = is_vaddr_t_user (&current);
			pd_entry->pt_base_address = (uint64_t)new_table / PAGE_SIZE;
			kmemset (get_vaddr_from_frame ((uint64_t)new_table / PAGE_SIZE), 0, PAGE_SIZE);
		}

		pt_entry_t* pt_base = (pt_entry_t*)get_vaddr_from_frame (pd_entry->pt_base_address);
		pt_entry_t* pt_entry = &pt_base[current.pt_index];

		pt_entry->present = 1;
		pt_entry->rw = 1;
		pt_entry->frame_base_address = phys_base_track / PAGE_SIZE;
		pt_entry->us = is_vaddr_t_user (&current);
		phys_base_track += PAGE_SIZE;

		if (!is_vaddr_t_lt (&current, &last)) break;

		current.pt_index++;
		if (current.pt_index >= 512) {
			current.pt_index = 0;
			current.pd_index++;
			if (current.pd_index >= 512) {
				current.pd_index = 0;
				current.pdpt_index++;
			}
		}
	}
}

/*!
 * Allocate multiple consecutive virtual pages
 * @param count number of consecutive pages to allocate
 * @return base virtual address of allocated pages
 */
void* alloc_vpages (size_t req_count, bool user) {
	// all memory allocations are currently under 2 pml4 entries. this is 512 gb of memory for
	// kernel and user each, which should be plenty for literally any use case of COS.
	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[user ? USER_PML4_IDX : KRNL_PML4_IDX];
	if (!pml4t_entry->present) return nullptr;

	size_t	 count_so_far = 0;
	uint64_t start_page_idx = 0;

	for (uint64_t i = 0; i < 512ull * 512ull * 512ull;) {
		uint16_t pdpt_idx = (i >> 18) & 0x1FF;
		uint16_t pd_idx = (i >> 9) & 0x1FF;
		uint16_t pt_idx = i & 0x1FF;

		pdpt_entry_t* pdpt_base =
			(pdpt_entry_t*)get_vaddr_from_frame (pml4t_entry->pdpt_base_address);
		pdpt_entry_t* pdpt_entry = &pdpt_base[pdpt_idx];

		if (!pdpt_entry->present) {
			// we found 512*512 consecutive free pages!
			if (count_so_far == 0) start_page_idx = i;
			uint64_t pages_left = 512ull * 512ull - (i % (512ull * 512ull)); // just in case

			if (count_so_far + pages_left >= req_count) {
				count_so_far = req_count;
				break;
			}

			count_so_far += pages_left;
			i += pages_left;
			continue;
		}

		pd_entry_t* pd_base = (pd_entry_t*)get_vaddr_from_frame (pdpt_entry->pd_base_address);
		pd_entry_t* pd_entry = &pd_base[pd_idx];

		if (!pd_entry->present) {
			// we found 512 consecutive free pages!
			if (count_so_far == 0) start_page_idx = i;
			uint64_t pages_left = 512ull - (i % 512ull); // just in case

			if (count_so_far + pages_left >= req_count) {
				count_so_far = req_count;
				break;
			}

			count_so_far += pages_left;
			i += pages_left;
			continue;
		}

		pt_entry_t* pt_base = (pt_entry_t*)get_vaddr_from_frame (pd_entry->pt_base_address);
		pt_entry_t* pt_entry = &pt_base[pt_idx];

		if (pt_entry->present) {
			count_so_far = 0;
			i++;
		} else {
			if (count_so_far == 0) start_page_idx = i;
			count_so_far++;
			if (count_so_far == req_count) break;
			i++;
		}
	}

	if (count_so_far == req_count) {
		paddr_t base_physical = alloc_ppages (req_count);
		if (base_physical == nullptr) return nullptr; // no more physical memory

		vaddr_t first_vaddr = {user ? USER_PML4_IDX : KRNL_PML4_IDX, (start_page_idx >> 18) & 0x1FF,
							   (start_page_idx >> 9) & 0x1FF, start_page_idx & 0x1FF, 0};
		vaddr_t last_vaddr = {user ? USER_PML4_IDX : KRNL_PML4_IDX,
							  ((start_page_idx + req_count - 1) >> 18) & 0x1FF,
							  ((start_page_idx + req_count - 1) >> 9) & 0x1FF,
							  (start_page_idx + req_count - 1) & 0x1FF, 0};

		alloc_all_vpages_in_range (first_vaddr, last_vaddr, base_physical);
		return vaddr_t_to_ptr (&first_vaddr);
	}

	return nullptr;
}

/*!
 * Allocate one virtual page
 * @return base virtual address of allocated page
 */
void* alloc_vpage (bool user) { return alloc_vpages (1, user); }

/*!
 * Check if page structure is empty
 * @param table_vaddr virtual address of the page structure
 * @return whether it is empty
 */
static bool is_table_empty (void* table_vaddr) {
	uint64_t* entries = (uint64_t*)table_vaddr;
	for (int i = 0; i < 512; i++)
		if (entries[i] & 1) return false;
	return true;
}

/*!
 * Frees all virtual pages in a given range (inclusive)
 * @param first first virtual page in range
 * @param last last virtual page in range
 */
void free_all_vpages_in_range (vaddr_t first, vaddr_t last) {
	vaddr_t current = first;
	while (is_vaddr_t_lt (&current, &last)) {
		pml4t_entry_t* pml4t_entry = &pml4_base_ptr[current.pml4_index];

		if (!pml4t_entry->present) {
			current.pml4_index++;
			current.pdpt_index = current.pd_index = current.pt_index = 0;
			continue;
		}

		pdpt_entry_t* pdpt_base =
			(pdpt_entry_t*)get_vaddr_from_frame (pml4t_entry->pdpt_base_address);
		pdpt_entry_t* pdpt_entry = &pdpt_base[current.pdpt_index];

		if (!pdpt_entry->present) {
			current.pdpt_index++;
			if (current.pdpt_index >= 512) current.pdpt_index -= 512, current.pml4_index++;
			current.pd_index = current.pt_index = 0;
			continue;
		}

		pd_entry_t* pd_base = (pd_entry_t*)get_vaddr_from_frame (pdpt_entry->pd_base_address);
		pd_entry_t* pd_entry = &pd_base[current.pd_index];

		if (!pd_entry->present) {
			current.pd_index++;
			if (current.pd_index >= 512) current.pd_index -= 512, current.pdpt_index++;
			if (current.pdpt_index >= 512) current.pdpt_index -= 512, current.pml4_index++;
			current.pt_index = 0;
			continue;
		}

		pt_entry_t* pt_base = (pt_entry_t*)get_vaddr_from_frame (pd_entry->pt_base_address);
		pt_entry_t* pt_entry = &pt_base[current.pt_index];

		if (pt_entry->present) {
			kserial_printf ("[free_all_vpages_in_range] Freeing pt_entry %03i with frame base addr "
							"0x%016llx (physical 0x%016llx\n",
							current.pt_index, vaddr_t_to_ptr (&current),
							pt_entry->frame_base_address * PAGE_SIZE);

			pt_entry->present = 0;
			free_ppage ((void*)(pt_entry->frame_base_address * PAGE_SIZE));
			pt_entry->frame_base_address = 0;
		}

		void* current_vaddr_ptr = vaddr_t_to_ptr (&current);
		__asm__ volatile ("invlpg (%0)" : : "r"(current_vaddr_ptr) : "memory");

		bool on_final_pg = !is_vaddr_t_lt (&current, &last);
		if (((current.pt_index == 511) || on_final_pg) && is_table_empty (pt_base)) {
			pd_entry->present = 0;
			free_ppage ((void*)(pd_entry->pt_base_address * PAGE_SIZE));
			pd_entry->pt_base_address = 0;

			if (((current.pd_index == 511) || on_final_pg) && is_table_empty (pd_base)) {
				pdpt_entry->present = 0;
				free_ppage ((void*)(pdpt_entry->pd_base_address * PAGE_SIZE));
				pdpt_entry->pd_base_address = 0;

				if (((current.pdpt_index == 511) || on_final_pg) && is_table_empty (pdpt_base)) {
					pml4t_entry->present = 0;
					free_ppage ((void*)(pml4t_entry->pdpt_base_address * PAGE_SIZE));
					pml4t_entry->pdpt_base_address = 0;
				}
			}
		}

		if (on_final_pg) break;

		current.pt_index++;
		if (current.pt_index >= 512) {
			current.pt_index = 0;
			current.pd_index++;
			if (current.pd_index >= 512) {
				current.pd_index = 0;
				current.pdpt_index++;
				if (current.pdpt_index >= 512) {
					current.pdpt_index = 0;
					current.pml4_index++;
				}
			}
		}
	}
}

/*!
 * Free multiple consecutive virtual pages=
 * @param ptr virtual address of the first page to free
 * @param count number of consecutive pages to free
 */
void free_vpages (void* ptr, size_t count) {
	if (ptr == nullptr || count == 0) return;

	vaddr_t vaddr = get_vaddr_t_from_ptr (ptr);
	if (vaddr.pml4_index != 1) return;

	void* phys_base = get_paddr (ptr);
	if (phys_base == nullptr) return;

	uint64_t start_ptr_64t = (uint64_t)ptr;
	uint64_t last_ptr_64t = start_ptr_64t + ((count - 1) * PAGE_SIZE);

	vaddr_t first_vaddr = get_vaddr_t_from_ptr ((void*)start_ptr_64t);
	vaddr_t last_vaddr = get_vaddr_t_from_ptr ((void*)last_ptr_64t);

	free_all_vpages_in_range (first_vaddr, last_vaddr);
}

/*!
 * Free single virtual page
 * @param ptr virtual address of the page to free
 */
void free_vpage (void* ptr) { free_vpages (ptr, 1); }

void alloc_by_cr3 (uint64_t cr3, uintptr_t start, size_t num_pages, bool write) {
	if (num_pages == 0) return;
	pml4t_entry_t* original_ptr = pml4_base_ptr;
	pml4_base_ptr = (pml4t_entry_t*)(cr3 + get_hhdm_offset ());
	uintptr_t last_page_base = ALIGN_PAGE_DOWN (start + (num_pages * PAGE_SIZE) - 1);

	paddr_t physmem = alloc_ppages (num_pages);
	alloc_all_vpages_in_range (get_vaddr_t_from_ptr ((void*)start),
							   get_vaddr_t_from_ptr ((void*)last_page_base), physmem);

	(void)write; // TODO: set rw flag

	pml4_base_ptr = original_ptr;
}

void dealloc_by_cr3 (uint64_t cr3, uintptr_t start, size_t num_pages) {
	if (num_pages == 0) return;
	pml4t_entry_t* original_ptr = pml4_base_ptr;
	pml4_base_ptr = (pml4t_entry_t*)(cr3 + get_hhdm_offset ());
	uintptr_t last_page_base = ALIGN_PAGE_DOWN (start + (num_pages * PAGE_SIZE) - 1);

	free_all_vpages_in_range (get_vaddr_t_from_ptr ((void*)start),
							  get_vaddr_t_from_ptr ((void*)last_page_base));
	pml4_base_ptr = original_ptr;
}

void init_vmm (pml4t_entry_t* kernel_pml4) { pml4_base_ptr = kernel_pml4; }
