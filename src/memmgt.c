#include <memmgt.h>
#include <stdio.h>
#include <memory.h>


pml4t_entry_t* pml4_base_ptr = NULL;
uint64_t hhdm_offset = 0;


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
void init_memmgt(uint64_t p_hhdm_offset) {
	uint64_t pml4_base = read_cr3() & 0xFFFFFFFFFF000;
	pml4_base_ptr = (pml4t_entry_t*)(pml4_base + p_hhdm_offset);
	hhdm_offset = p_hhdm_offset;
}

/*!
 * Walks the page table hierarchy and prints present entries and their address ranges.
 */
void walk_pagetable() {
	for (int i = 0; i < 512; i++) {
		pml4t_entry_t* pml4t_entry = &pml4_base_ptr[i];
		if (!pml4t_entry->present) continue;

		printf("PML4 %d:\tPDPT Base Address: 0x%lx\tVaddr: 0x%lx\n",
			i, pml4t_entry->pdpt_base_address << 12,
			(pml4t_entry->pdpt_base_address << 12)+hhdm_offset);

		pdpt_entry_t* pdpt_base_ptr = (pdpt_entry_t*)((pml4t_entry->pdpt_base_address << 12) + hhdm_offset);
		for (int j = 0; j < 512; j++) {
			pdpt_entry_t* pdpt_entry = &pdpt_base_ptr[j];
			if (!pdpt_entry->present) continue;

			printf("  PDPT %d:\tPD Base Address:   0x%lx\tVaddr: 0x%lx\n",
				j, pdpt_entry->pd_base_address << 12,
				(pdpt_entry->pd_base_address << 12) + hhdm_offset);

			pd_entry_t* pd_base_ptr = (pd_entry_t*)((pdpt_entry->pd_base_address << 12) + hhdm_offset);
			bool is_present_pd[512] = { false };
			for (int k = 0; k < 512; k++) is_present_pd[k] = (&pd_base_ptr[k])->present;

			// print ranges of present pd's
			int range_start = -1;
			for (int k = 0; k <= 512; k++) {
				if (k < 512 && is_present_pd[k]) {
					if (range_start == -1)
						range_start = k;
				}
				else {
					if (range_start != -1) {
						if (range_start == k - 1) {
							printf("    Present PD: %d\n", range_start);
						}
						else {
							printf("    Present PDs: %d-%d\n", range_start, k - 1);
						}
						range_start = -1;
					}
				}
			}
		}
	}
}

/*!
 * Finds the virtual address mapped to a given physical address.
 * @param paddr The physical address to look up.
 * @return The corresponding virtual address, or NULL if not mapped.
 */
void* get_vaddr(void* paddr) {
	uint64_t phys_addr = (uint64_t)paddr;
	uint64_t target_frame = phys_addr >> 12;

	for (int i = 0; i < 512; i++) {
		pml4t_entry_t* pml4t_entry = &pml4_base_ptr[i];
		if (!pml4t_entry->present) continue;

		pdpt_entry_t* pdpt_base_ptr = (pdpt_entry_t*)((pml4t_entry->pdpt_base_address << 12) + hhdm_offset);

		for (int j = 0; j < 512; j++) {
			pdpt_entry_t* pdpt_entry = &pdpt_base_ptr[j];
			if (!pdpt_entry->present) continue;

			pd_entry_t* pd_base_ptr = (pd_entry_t*)((pdpt_entry->pd_base_address << 12) + hhdm_offset);

			for (int k = 0; k < 512; k++) {
				pd_entry_t* pd_entry = &pd_base_ptr[k];
				if (!pd_entry->present) continue;

				pt_entry_t* pt_base_ptr = (pt_entry_t*)((pd_entry->pt_base_address << 12) + hhdm_offset);

				for (int l = 0; l < 512; l++) {
					pt_entry_t* pt_entry = &pt_base_ptr[l];
					if (!pt_entry->present) continue;

					if (pt_entry->frame_base_address == target_frame) {
						uint64_t vaddr = 0;
						vaddr |= ((uint64_t)i << 39);
						vaddr |= ((uint64_t)j << 30);
						vaddr |= ((uint64_t)k << 21);
						vaddr |= ((uint64_t)l << 12);
						vaddr |= (phys_addr & 0xFFF);

						return (void*)vaddr;
					}
				}
			}
		}
	}

	// Physical address not found in any mapped page
	return NULL;
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