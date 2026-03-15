#include <kernel/limine.h>
#include <kernel/memmgt.h>
#include <kernel/serial.h>
#include <memory.h>
#include <stdio.h>

#define PAGE_SIZE 4096

uint64_t alloc_frames_base = 0;
uint64_t alloc_frames_count = 0;
uint64_t alloc_frames_limit = 0;

pml4t_entry_t* pml4_base_ptr = NULL;
uint64_t hhdm_offset = 0;

bool is_locked = false;

memmap_bitmap bitmap;

extern struct {
	pdpt_entry_t pdpt_entry[512];
	pd_entry_t pd_entry[512];
	pt_entry_t pt_entry[512];
} memmap;

/*!
 * Reads the value of the CR3 register, which contains the physical address of the PML4 table.
 * @return The value of the CR3 register.
 */
uint64_t read_cr3 () {
	uint64_t cr3;
	__asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
	return cr3;
}

/*!
 * Get a vaddr_t object from a virtual pointer
 * @param ptr the virtual pointer
 * @return corresponding vaddr_t object
 */
vaddr_t get_vaddr_t_from_ptr (void* ptr) {
	uint64_t ptr_64t = (uint64_t)ptr;
	vaddr_t ret_vaddr;

	ret_vaddr.offset = ptr_64t & 0xFFF;
	ret_vaddr.pt_index = (ptr_64t >> 12) & 0x1FF;
	ret_vaddr.pd_index = (ptr_64t >> 21) & 0x1FF;
	ret_vaddr.pdpt_index = (ptr_64t >> 30) & 0x1FF;
	ret_vaddr.pml4_index = (ptr_64t >> 39) & 0x1FF;

	return ret_vaddr;
}

/*!
 * Convert a physical frame address to vaddr pointer with HHDM mapping
 * @param phys_address the physical address
 * @return pointer to virtual memory using HHDM mapping
 */
inline void* get_vaddr_hhdm (uint64_t phys_address) {
	return (void*)((phys_address << 12) + hhdm_offset);
}

/*!
 * Set a bit in the memory bitmap
 * @param page_idx page index
 */
static inline void bitmap_set_bit(uint64_t page_idx) {
    bitmap.map[page_idx / 8] |= (1 << (page_idx % 8));
    bitmap.pages_used++;
}

/*!
 * Clear a bit in the memory bitmap
 * @param page_idx page index
 */
static inline void bitmap_clear_bit(uint64_t page_idx) {
    if (bitmap.map[page_idx / 8] & (1 << (page_idx % 8))) {
        bitmap.map[page_idx / 8] &= ~(1 << (page_idx % 8));
        bitmap.pages_used--;
    }
}

paddr_t alloc_ppage () {
	if (bitmap.pages_used >= bitmap.pages_maxlen)
		return NULL;

	uint64_t total_bytes = (bitmap.pages_maxlen + 7) / 8;
	for (uint64_t i = 0; i < total_bytes; i++) {
		if (bitmap.map[i] == 0xFF)
			continue;
		for (int bit = 0; bit < 8; bit++) {
			uint64_t idx = i * 8 + bit;
			if (idx >= bitmap.pages_maxlen)
				return NULL;
			if (!(bitmap.map[i] & (1 << bit))) {
				bitmap.map[i] |= (1 << bit);
				bitmap.pages_used++;
				return (paddr_t)(bitmap.pages_base + PAGE_SIZE * idx);
			}
		}
	}
	return NULL;
}

static void init_physical_bitmap (struct limine_memmap_response* memmap_response) {
	uint64_t addr_limit = 0;

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			uint64_t top = entry->base + entry->length;
			if (top > addr_limit) addr_limit = top;
		}
	}

	uint64_t total_pages = addr_limit / PAGE_SIZE;
	uint64_t bitmap_size = (total_bytes + 7) / 8;

	void* bitmap_phys_addr = NULL;
	for (uint64_t i=0; i<memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
			bitmap_phys_addr = (void*) entry->base;
			break;
		}
	}

	if (bitmap_phys_addr == NULL) {
		write_serial_str ("Not enough contiguous memory for bitmap setup!! Please download some RAM.\n");
		printf("Not enough contiguous memory for bitmap setup!! Please download some RAM.\n");
		__asm__ ("hlt");
	}

	bitmap.map = (uint8_t*) get_vaddr_hhdm ((uint64_t)bitmap_phys_addr);
	bitmap.pages_base = 0;
	bitmap.pages_maxlen = total_pages;
	bitmap.pages_used = total_pages;

	memset(bitmap.map, 0xFF, bitmap_size);

	uint64_t bitmap_fst_page = (uint64_t) bitmap_phys_addr / PAGE_SIZE;
	uint64_t bitmap_lst_page = bitmap_fst_page + ((bitmap_size + PAGE_SIZE -1)/PAGE_SIZE);

	for (uint64_t i = 0; i<memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			for (uint64_t p = entry->base / PAGE_SIZE ; p < (entry->base + entry->length) / PAGE_SIZE ; p++) {
				if (p < bitmap_fst_page || p >= bitmap_end_page) bitmap_clear_bit(p);
			}
		}
	}
}

/*!
 * Initializes the memory management subsystem.
 * Sets the base pointer for the PML4 table and stores the HHDM offset.
 * @param p_hhdm_offset The higher half direct mapping offset.
 */
void init_memmgt (uint64_t p_hhdm_offset, struct limine_memmap_response* memmap_response) {
	uint64_t pml4_base = read_cr3 () & 0xFFFFFFFFFF000;
	pml4_base_ptr = (pml4t_entry_t*)(pml4_base + p_hhdm_offset);
	hhdm_offset = p_hhdm_offset;

	uint64_t highest_length_so_far = 0;
	uint64_t best_base_so_far = 0xffffffffffffffff;

	// set permissible limit
	if (memmap_response != NULL) {
		for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
			if (memmap_response->entries[i]->type == 0) {
				if (memmap_response->entries[i]->length > highest_length_so_far) {
					highest_length_so_far = memmap_response->entries[i]->length;
					best_base_so_far = memmap_response->entries[i]->base;
				}
			}
		}
	}

	if (highest_length_so_far < PAGE_SIZE * 16) {
		printf ("Too little memory!! (0x%lx bytes)\n", highest_length_so_far);
		return;
	}

	alloc_frames_base = best_base_so_far;
	alloc_frames_limit = highest_length_so_far + best_base_so_far;

	// set up bitmap for physical page allocation
	init_physical_bitmap (memmap_response);

	// set up default memory map
	memset (&memmap, 0, sizeof (memmap));

	for (int i = 0; i < 1; i++) {
		memmap.pdpt_entry[i].present = 1;
		memmap.pdpt_entry[i].read_write = 1;
		memmap.pdpt_entry[i].pd_base_address =
			((uint64_t)(get_paddr (&memmap.pd_entry[i * 512])) >> 12) & 0xFFFFFFFFFF;
		memmap.pdpt_entry[i].xd = 1;
	}

	for (int i = 0; i < 1; i++) {
		memmap.pd_entry[i].present = 1;
		memmap.pd_entry[i].rw = 1;
		memmap.pd_entry[i].nex = 1;
		memmap.pd_entry[i].pt_base_address =
			((uint64_t)(get_paddr (&memmap.pt_entry[i * 512])) >> 12) & 0xFFFFFFFFFF;
	}

	for (int i = 0; i < 512; i++) {
		memmap.pt_entry[i].present = 1;
		memmap.pt_entry[i].rw = 1;
		memmap.pt_entry[i].frame_base_address =
			((uint64_t)(alloc_frames_base + PAGE_SIZE * i) >> 12) & 0xFFFFFFFFFF;
	}

	// initialise PML4T idx 1 and PDPT idx 0 for our page assignments
	uint64_t pdpt_base_address = ((uint64_t)(get_paddr (&memmap.pdpt_entry[0])));
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

uint64_t allocate_physical_pageframes (size_t count) {
	if (alloc_frames_base + (PAGE_SIZE * (alloc_frames_count + count)) > alloc_frames_limit)
		return 0;
	if (is_locked)
		return 0;

	uint64_t allocated_memory = alloc_frames_base + (PAGE_SIZE * alloc_frames_count);
	alloc_frames_count += count;
	return allocated_memory;
}

/*!
 * Walks the page table hierarchy and prints present entries and their address ranges.
 */
void walk_pagetable () {
	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[1];
	if (!pml4t_entry->present)
		return;

	pdpt_entry_t* pdpt_base_ptr = (pdpt_entry_t*)get_vaddr_hhdm (pml4t_entry->pdpt_base_address);
	pdpt_entry_t* pdpt_entry = &pdpt_base_ptr[0];
	if (!pdpt_entry->present)
		return;

	pd_entry_t* pd_base_ptr = (pd_entry_t*)get_vaddr_hhdm (pdpt_entry->pd_base_address);
	for (int k = 0; k < 512; k++) {
		pd_entry_t* pd_entry = &pd_base_ptr[k];
		if (!pd_entry->present)
			continue;
		printf ("PD %d: PT Base Address:   0x%lx\n", k, pd_entry->pt_base_address << 12);
		pt_entry_t* pt_base_ptr = (pt_entry_t*)get_vaddr_hhdm (pd_entry->pt_base_address);

		bool is_present_pt[512] = {false};
		for (int k = 0; k < 512; k++)
			is_present_pt[k] = (&pt_base_ptr[k])->present;

		// print ranges of present pd's
		int range_start = -1;
		for (int k = 0; k <= 512; k++) {
			if (k < 512 && is_present_pt[k]) {
				if (range_start == -1)
					range_start = k;
			} else {
				if (range_start != -1) {
					if (range_start == k - 1) {
						printf ("  Present PT: %d\n", range_start);
					} else {
						printf ("  Present PTs: %d-%d\n", range_start, k - 1);
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
void* get_paddr (void* vaddr) {
	vaddr_t virtual_addr = get_vaddr_t_from_ptr (vaddr);

	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[virtual_addr.pml4_index];
	if (!pml4t_entry->present)
		return NULL;

	pdpt_entry_t* pdpt_base_ptr = (pdpt_entry_t*)get_vaddr_hhdm (pml4t_entry->pdpt_base_address);
	pdpt_entry_t* pdpt_entry = &pdpt_base_ptr[virtual_addr.pdpt_index];
	if (!pdpt_entry->present)
		return NULL;

	pd_entry_t* pd_base_ptr = (pd_entry_t*)get_vaddr_hhdm (pdpt_entry->pd_base_address);
	pd_entry_t* pd_entry = &pd_base_ptr[virtual_addr.pd_index];
	if (!pd_entry->present)
		return NULL;

	pt_entry_t* pt_base_ptr = (pt_entry_t*)get_vaddr_hhdm (pd_entry->pt_base_address);
	pt_entry_t* pt_entry = &pt_base_ptr[virtual_addr.pt_index];
	if (!pt_entry->present)
		return NULL;

	uint64_t phys_addr = (pt_entry->frame_base_address << 12) | virtual_addr.offset;

	return (void*)phys_addr;
}

/*

LIBALLOC FUNCTION IMPLEMENTATIONS

*/

void* try_assign_pt (pt_entry_t* pt_base_ptr, size_t count) {
	int free_count = 0;
	for (int i = 0; i < 512; i++) {
		if (pt_base_ptr[i].allocated) {
			free_count = 0;
			continue;
		} else {
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

int liballoc_lock () {
	is_locked = true;
	return 0;
}

int liballoc_unlock () {
	is_locked = false;
	return 0;
}

void* liballoc_alloc (size_t count) {
	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[1];
	if (!pml4t_entry->present)
		return NULL;
	pdpt_entry_t* pdpt_entry = &((pdpt_entry_t*)get_vaddr_hhdm (pml4t_entry->pdpt_base_address))[0];
	if (!pdpt_entry->present)
		return NULL;
	pd_entry_t* pd_entry = &((pd_entry_t*)get_vaddr_hhdm (pdpt_entry->pd_base_address))[0];
	if (!pd_entry->present)
		return NULL;
	pt_entry_t* pt_base_ptr = (pt_entry_t*)get_vaddr_hhdm (pd_entry->pt_base_address);

	return try_assign_pt (pt_base_ptr, count);
}

int liballoc_free (void* ptr, size_t count) {
	if (is_locked)
		return -7;
	if (get_paddr (ptr) == NULL)
		return -1;

	vaddr_t vaddr = get_vaddr_t_from_ptr (ptr);

	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[vaddr.pml4_index];
	if (!pml4t_entry->present || vaddr.pml4_index != 1)
		return -2;
	pdpt_entry_t* pdpt_entry =
		&((pdpt_entry_t*)get_vaddr_hhdm (pml4t_entry->pdpt_base_address))[vaddr.pdpt_index];
	if (!pdpt_entry->present || vaddr.pdpt_index != 0)
		return -3;
	pd_entry_t* pd_entry =
		&((pd_entry_t*)get_vaddr_hhdm (pdpt_entry->pd_base_address))[vaddr.pd_index];
	if (!pd_entry->present || vaddr.pd_index != 0)
		return -4;
	pt_entry_t* pt_base_ptr = (pt_entry_t*)get_vaddr_hhdm (pd_entry->pt_base_address);

	for (size_t i = 0; i < count; i++) {
		if (i + vaddr.pt_index >= 512)
			return -5; // out of bounds

		pt_entry_t* pt_entry = &pt_base_ptr[i + vaddr.pt_index];
		if (!pt_entry->allocated)
			return -6;

		pt_entry->allocated = 0;
	}

	return 0;
}