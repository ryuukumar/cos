#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/memmgt.h>
#include <kernel/memory/pmm.h>

static paddr_t clone_pframes (paddr_t p_src, uint64_t page_count) {
	uint64_t hhdm_offset = get_hhdm_offset ();

	paddr_t p_dest = alloc_ppages (page_count);
	if (!p_dest) return nullptr;

	void* v_dest = (void*)((uint64_t)p_dest + hhdm_offset);
	void* v_src = (void*)((uint64_t)p_src + hhdm_offset);
	kmemcpy (v_dest, v_src, page_count * PAGE_SIZE);
	return p_dest;
}

/*!
 * Deep clones memory structure of one process to a new process' page table structures. Useful for
 * fork() calls.
 * @param cr3_src CR3 of the source process (e.g. fork caller)
 * @param cr3_dest Pointer to set CR3 of the new process
 * @return 0 if successful, else error code
 */
int clone_user_memory (uint64_t cr3_src, uint64_t* cr3_dest) {
	uint64_t hhdm_offset = get_hhdm_offset ();

	pml4t_entry_t* src_pml4_table = (pml4t_entry_t*)(cr3_src + hhdm_offset);
	paddr_t		   dest_pml4_phys = alloc_ppage ();
	if (!dest_pml4_phys) return -ENOMEM;

	pml4t_entry_t* dest_pml4_table = (pml4t_entry_t*)((uint64_t)dest_pml4_phys + hhdm_offset);
	kmemset (dest_pml4_table, 0, PAGE_SIZE);

	// map the higher half to be exactly the same as kernel's
	pml4t_entry_t* krnl_pml4_table = (pml4t_entry_t*)((uint64_t)get_kernel_cr3 () + hhdm_offset);
	kmemcpy (&dest_pml4_table[256], &krnl_pml4_table[256], 256 * sizeof (pml4t_entry_t));

	// deep clone the lower half (user space)
	for (uint16_t pml4_index = 0; pml4_index < 256; pml4_index++) {
		pml4t_entry_t* src_pml4_entry = &src_pml4_table[pml4_index];
		if (!src_pml4_entry->present) continue;
		// leaving out huge page clones at pml4 level, because there is no way we are going to have
		// 512gb pages (yet)!!!!

		paddr_t dest_pdpt_phys = alloc_ppage ();
		if (!dest_pdpt_phys) return -ENOMEM;
		pdpt_entry_t* dest_pdpt = (pdpt_entry_t*)((uint64_t)dest_pdpt_phys + hhdm_offset);
		kmemset (dest_pdpt, 0, PAGE_SIZE);

		dest_pml4_table[pml4_index] = *src_pml4_entry;
		dest_pml4_table[pml4_index].pdpt_base_address = (uint64_t)dest_pdpt_phys / PAGE_SIZE;

		pdpt_entry_t* src_pdpt =
			(pdpt_entry_t*)get_vaddr_from_frame (src_pml4_entry->pdpt_base_address);

		for (uint16_t pdpt_idx = 0; pdpt_idx < 512; pdpt_idx++) {
			pdpt_entry_t* src_pdpt_entry = &src_pdpt[pdpt_idx];
			if (!src_pdpt_entry->present) continue;
			if (src_pdpt_entry->page_size) {
				paddr_t dest_frame_phys =
					clone_pframes ((paddr_t)(src_pdpt_entry->pd_base_address * PAGE_SIZE), 262144);
				if (!dest_frame_phys) return -ENOMEM;

				dest_pdpt[pdpt_idx] = *src_pdpt_entry;
				dest_pdpt[pdpt_idx].pd_base_address = (uint64_t)dest_frame_phys / PAGE_SIZE;
				continue;
			}

			paddr_t dest_pd_phys = alloc_ppage ();
			if (!dest_pd_phys) return -ENOMEM;
			pd_entry_t* dest_pd = (pd_entry_t*)((uint64_t)dest_pd_phys + hhdm_offset);
			kmemset (dest_pd, 0, PAGE_SIZE);

			dest_pdpt[pdpt_idx] = *src_pdpt_entry;
			dest_pdpt[pdpt_idx].pd_base_address = (uint64_t)dest_pd_phys / PAGE_SIZE;

			pd_entry_t* src_pd =
				(pd_entry_t*)get_vaddr_from_frame (src_pdpt_entry->pd_base_address);

			for (uint16_t pd_idx = 0; pd_idx < 512; pd_idx++) {
				pd_entry_t* src_pd_entry = &src_pd[pd_idx];
				if (!src_pd_entry->present) continue;

				if (src_pd_entry->page_size) {
					paddr_t dest_frame_phys =
						clone_pframes ((paddr_t)(src_pd_entry->pt_base_address * PAGE_SIZE), 512);
					if (!dest_frame_phys) return -ENOMEM;

					dest_pd[pd_idx] = *src_pd_entry;
					dest_pd[pd_idx].pt_base_address = (uint64_t)dest_frame_phys / PAGE_SIZE;
					continue;
				}

				paddr_t dest_pt_phys = alloc_ppage ();
				if (!dest_pt_phys) return -ENOMEM;
				pt_entry_t* dest_pt = (pt_entry_t*)((uint64_t)dest_pt_phys + hhdm_offset);
				kmemset (dest_pt, 0, PAGE_SIZE);

				dest_pd[pd_idx] = *src_pd_entry;
				dest_pd[pd_idx].pt_base_address = (uint64_t)dest_pt_phys / PAGE_SIZE;

				pt_entry_t* src_pt =
					(pt_entry_t*)get_vaddr_from_frame (src_pd_entry->pt_base_address);

				for (uint16_t pt_idx = 0; pt_idx < 512; pt_idx++) {
					pt_entry_t* src_pt_entry = &src_pt[pt_idx];
					if (!src_pt_entry->present) continue;

					paddr_t dest_frame_phys =
						clone_pframes ((paddr_t)(src_pt_entry->frame_base_address * PAGE_SIZE), 1);
					if (!dest_frame_phys) return -ENOMEM;

					dest_pt[pt_idx] = *src_pt_entry;
					dest_pt[pt_idx].frame_base_address = (uint64_t)dest_frame_phys / PAGE_SIZE;
				}
			}
		}
	}

	*cr3_dest = (uint64_t)dest_pml4_phys;
	return 0;
}
