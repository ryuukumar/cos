#include <kernel/idt.h>
#include <kernel/limine.h>
#include <kernel/memmgt.h>
#include <kernel/serial.h>
#include <memory.h>
#include <stdio.h>

#define PAGE_SIZE 4096ull

pml4t_entry_t* pml4_base_ptr = NULL;
uint64_t hhdm_offset = 0;

bool is_locked = false;

memmap_bitmap bitmap;

struct limine_memmap_response* memmap_response_ptr;

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
 * Get a virtual pointer from a vaddr_t object. Automatically performs sign extension required in
 * x86_64
 * @param virtual_addr the vaddr_t object
 * @return corresponding virtual pointer
 */
inline void* vaddr_t_to_ptr (vaddr_t* virtual_addr) {
	uint64_t ptr_64t = ((uint64_t)virtual_addr->pml4_index << 39) |
					   ((uint64_t)virtual_addr->pdpt_index << 30) |
					   ((uint64_t)virtual_addr->pd_index << 21) |
					   ((uint64_t)virtual_addr->pt_index << 12) | (uint64_t)virtual_addr->offset;

	if (ptr_64t & (1ULL << 47))
		ptr_64t |= 0xFFFF000000000000ULL;
	return (void*)ptr_64t;
}

/*!
 * Convert a physical frame to vaddr pointer with HHDM mapping
 * @param phys_frame the physical frame
 * @return pointer to virtual memory using HHDM mapping
 */
inline void* get_vaddr_from_frame (uint64_t phys_frame) {
	return (void*)((phys_frame << 12) + hhdm_offset);
}

/*!
 * Set a bit in the memory bitmap
 * @param page_idx page index
 */
static inline void bitmap_set_bit (uint64_t page_idx) {
	bitmap.map[page_idx / 8] |= (1 << (page_idx % 8));
	bitmap.pages_used++;
}

/*!
 * Clear a bit in the memory bitmap
 * @param page_idx page index
 */
static inline void bitmap_clear_bit (uint64_t page_idx) {
	if (bitmap.map[page_idx / 8] & (1 << (page_idx % 8))) {
		bitmap.map[page_idx / 8] &= ~(1 << (page_idx % 8));
		bitmap.pages_used--;
	}
}

/*!
 * Allocate multiple consecutive physical frames
 * @param count number of consecutive frames to allocate
 * @return base physical address of allocated frames
 */
paddr_t alloc_ppages (uint64_t count) {
	if (count == 0)
		return NULL;
	if (bitmap.pages_used + count > bitmap.pages_maxlen)
		return NULL;

	uint64_t current_streak = 0;
	uint64_t start_idx = 0;

	for (uint64_t i = 0; i < bitmap.pages_maxlen; i++) {
		if (i % 8 == 0 && current_streak == 0 && bitmap.map[i / 8] == 0xFF) {
			i += 7;
			continue;
		}

		if ((bitmap.map[i / 8] & (1 << (i % 8))) == 0) {
			if (current_streak == 0)
				start_idx = i;
			current_streak++;

			if (current_streak == count) {
				for (uint64_t j = start_idx; j < start_idx + count; j++)
					bitmap_set_bit (j);
				return (paddr_t)(bitmap.pages_base + PAGE_SIZE * start_idx);
			}
		} else
			current_streak = 0;
	}

	return NULL;
}

/*!
 * Allocate a single physical frame
 * @return base physical address of allocated frame
 */
paddr_t alloc_ppage () { return alloc_ppages (1); }

/*!
 * Free multiple consecutive physical frames
 * @param paddr base physical address of frames to free
 * @param count number of frames to free
 */
void free_ppages (void* paddr, uint64_t count) {
	uint64_t start_idx = (uint64_t)paddr / PAGE_SIZE;

	for (uint64_t i = 0; i < count; i++) {
		uint64_t current_paddr = (uint64_t)paddr + (i * PAGE_SIZE);
		bool is_valid = false;

		if (memmap_response_ptr != NULL) {
			for (uint64_t j = 0; j < memmap_response_ptr->entry_count; j++) {
				struct limine_memmap_entry* entry = memmap_response_ptr->entries[j];
				if (entry->type == LIMINE_MEMMAP_USABLE && current_paddr >= entry->base &&
					current_paddr < (entry->base + entry->length)) {
					is_valid = true;
					break;
				}
			}
		}

		if (!is_valid) {
			char buffer[16];
			ulitos ((uint64_t)paddr, buffer, 16);
			write_serial_str ("Tried to free reserved memory! Address: 0x");
			write_serial_str (buffer);
			write_serial_str ("\nHalting.\n");
			__asm__ ("hlt");
		}

		bitmap_clear_bit (start_idx + i);
	}
}

/*!
 * Free a single physical frame
 * @param paddr base physical address of frame to free
 */
void free_ppage (void* paddr) { free_ppages (paddr, 1); }

static void init_physical_bitmap (struct limine_memmap_response* memmap_response) {
	uint64_t addr_limit = 0;

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			uint64_t top = entry->base + entry->length;
			if (top > addr_limit)
				addr_limit = top;
		}
	}

	uint64_t total_pages = addr_limit / PAGE_SIZE;
	uint64_t bitmap_size = (total_pages + 7) / 8;

	void* bitmap_phys_addr = NULL;
	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
			bitmap_phys_addr = (void*)entry->base;
			break;
		}
	}

	if (bitmap_phys_addr == NULL) {
		write_serial_str (
			"Not enough contiguous memory for bitmap setup!! Please download some RAM.\n");
		printf ("Not enough contiguous memory for bitmap setup!! Please download some RAM.\n");
		__asm__ ("hlt");
	}

	bitmap.map = (uint8_t*)((uint64_t)bitmap_phys_addr + hhdm_offset);
	bitmap.pages_base = 0;
	bitmap.pages_maxlen = total_pages;
	bitmap.pages_used = total_pages;

	memset (bitmap.map, 0xFF, bitmap_size);

	uint64_t bitmap_fst_page = (uint64_t)bitmap_phys_addr / PAGE_SIZE;
	uint64_t bitmap_lst_page = bitmap_fst_page + ((bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE);

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			for (uint64_t p = entry->base / PAGE_SIZE;
				 p < (entry->base + entry->length) / PAGE_SIZE; p++) {
				if (p < bitmap_fst_page || p >= bitmap_lst_page)
					bitmap_clear_bit (p);
			}
		}
	}
}

/*!
 * Simple handler for page fault, prints faulting address from CR2
 * @param registers idt-passed registers object
 */
void page_fault_handler (registers_t* registers) {
	uint64_t cr2;
	__asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));

	char buffer[16];
	ulitos (cr2, buffer, 16);

	write_serial_str ("\nEncountered a page fault!\n");
	write_serial_str ("Faulting address: 0x");
	write_serial_str (buffer);
	write_serial ('\n');

	for (;;)
		__asm__ volatile ("hlt");
}

/*!
 * Check if vaddr a is strictly less than vaddr b
 * @param a first vaddr
 * @param b second vaddr
 * @return true if a lies before b, false otherwise
 */
static bool is_vaddr_t_lt (vaddr_t* a, vaddr_t* b) {
	if (a->pml4_index == b->pml4_index) {
		if (a->pdpt_index == b->pdpt_index) {
			if (a->pd_index == b->pd_index) {
				if (a->pt_index == b->pt_index)
					return a->offset < b->offset;
				return a->pt_index < b->pt_index;
			}
			return a->pd_index < b->pd_index;
		}
		return a->pdpt_index < b->pdpt_index;
	}
	return a->pml4_index < b->pml4_index;
}

/*!
 * Allocate all virtual pages in given range (inclusive). Needs physical memory to be allocated.
 * @param first first virtual address in range
 * @param last last virtual address in range
 * @param base_addr base address of physical memory of corresponding size
 */
static void alloc_all_vpages_in_range (vaddr_t first, vaddr_t last, paddr_t base_addr, bool user) {
	uint64_t phys_base_track = (uint64_t)base_addr;
	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[first.pml4_index];

	vaddr_t current = first;

	while (true) {
		pdpt_entry_t* pdpt_base =
			(pdpt_entry_t*)get_vaddr_from_frame (pml4t_entry->pdpt_base_address);
		pdpt_entry_t* pdpt_entry = &pdpt_base[current.pdpt_index];

		if (!pdpt_entry->present) {
			paddr_t new_table = alloc_ppage ();
			pdpt_entry->present = 1;
			pdpt_entry->read_write = 1;
			pdpt_entry->pd_base_address = (uint64_t)new_table / PAGE_SIZE;
			memset (get_vaddr_from_frame ((uint64_t)new_table / PAGE_SIZE), 0, PAGE_SIZE);
		}
		pdpt_entry->user_supervisor = user;

		pd_entry_t* pd_base = (pd_entry_t*)get_vaddr_from_frame (pdpt_entry->pd_base_address);
		pd_entry_t* pd_entry = &pd_base[current.pd_index];

		if (!pd_entry->present) {
			paddr_t new_table = alloc_ppage ();
			pd_entry->present = 1;
			pd_entry->rw = 1;
			pd_entry->pt_base_address = (uint64_t)new_table / PAGE_SIZE;
			memset (get_vaddr_from_frame ((uint64_t)new_table / PAGE_SIZE), 0, PAGE_SIZE);
		}
		pd_entry->us = user;

		pt_entry_t* pt_base = (pt_entry_t*)get_vaddr_from_frame (pd_entry->pt_base_address);
		pt_entry_t* pt_entry = &pt_base[current.pt_index];

		pt_entry->present = 1;
		pt_entry->rw = 1;
		pt_entry->frame_base_address = phys_base_track / PAGE_SIZE;
		pt_entry->us = user;
		phys_base_track += PAGE_SIZE;

		if (!is_vaddr_t_lt (&current, &last))
			break;

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
	// all memory allocations are currently under one pml4 entry. this is 512 gb of memory, which
	// should be plenty for literally any use case of COS.
	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[1];
	if (!pml4t_entry->present)
		return NULL;

	size_t count_so_far = 0;
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
			if (count_so_far == 0)
				start_page_idx = i;
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
			if (count_so_far == 0)
				start_page_idx = i;
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
			if (count_so_far == 0)
				start_page_idx = i;
			count_so_far++;
			if (count_so_far == req_count)
				break;
			i++;
		}
	}

	if (count_so_far == req_count) {
		paddr_t base_physical = alloc_ppages (req_count);
		if (base_physical == NULL)
			return NULL; // no more physical memory

		vaddr_t first_vaddr = {1, (start_page_idx >> 18) & 0x1FF, (start_page_idx >> 9) & 0x1FF,
							   start_page_idx & 0x1FF, 0};
		vaddr_t last_vaddr = {1, ((start_page_idx + req_count - 1) >> 18) & 0x1FF,
							  ((start_page_idx + req_count - 1) >> 9) & 0x1FF,
							  (start_page_idx + req_count - 1) & 0x1FF, 0};

		alloc_all_vpages_in_range (first_vaddr, last_vaddr, base_physical, user);
		return vaddr_t_to_ptr (&first_vaddr);
	}

	return NULL;
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
		if (entries[i] & 1)
			return false;
	return true;
}

/*!
 * Frees all virtual pages in a given range (inclusive)
 * @param first first virtual page in range
 * @param last last virtual page in range
 */
static void free_all_vpages_in_range (vaddr_t first, vaddr_t last) {
	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[first.pml4_index];
	if (!pml4t_entry->present)
		return;

	vaddr_t current = first;

	while (true) {
		pdpt_entry_t* pdpt_base =
			(pdpt_entry_t*)get_vaddr_from_frame (pml4t_entry->pdpt_base_address);
		pdpt_entry_t* pdpt_entry = &pdpt_base[current.pdpt_index];

		if (pdpt_entry->present) {
			pd_entry_t* pd_base = (pd_entry_t*)get_vaddr_from_frame (pdpt_entry->pd_base_address);
			pd_entry_t* pd_entry = &pd_base[current.pd_index];

			if (pd_entry->present) {
				pt_entry_t* pt_base = (pt_entry_t*)get_vaddr_from_frame (pd_entry->pt_base_address);
				pt_entry_t* pt_entry = &pt_base[current.pt_index];

				if (pt_entry->present) {
					pt_entry->present = 0;
					pt_entry->frame_base_address = 0;

					void* current_vaddr_ptr = vaddr_t_to_ptr (&current);
					__asm__ volatile ("invlpg (%0)" : : "r"(current_vaddr_ptr) : "memory");

					// --- GARBAGE COLLECTION ---
					// We only need to check if a table is empty when we cross its boundary OR on
					// our final page.
					bool on_final_pg = !is_vaddr_t_lt (&current, &last);
					if (((current.pt_index == 511) || on_final_pg) && is_table_empty (pt_base)) {
						free_ppage ((void*)(pd_entry->pt_base_address * PAGE_SIZE));
						pd_entry->present = 0;
						pd_entry->pt_base_address = 0;

						if (((current.pd_index == 511) || on_final_pg) &&
							is_table_empty (pd_base)) {
							free_ppage ((void*)(pdpt_entry->pd_base_address * PAGE_SIZE));
							pdpt_entry->present = 0;
							pdpt_entry->pd_base_address = 0;
						}
					}
				}
			}
		}

		if (!is_vaddr_t_lt (&current, &last))
			break;

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
 * Free multiple consecutive virtual pages=
 * @param ptr virtual address of the first page to free
 * @param count number of consecutive pages to free
 */
void free_vpages (void* ptr, size_t count) {
	if (ptr == NULL || count == 0)
		return;

	vaddr_t vaddr = get_vaddr_t_from_ptr (ptr);
	if (vaddr.pml4_index != 1) {
		write_serial_str ("free_vpages called for pml4_index that is not 1!");
		return;
	}

	void* phys_base = get_paddr (ptr);
	if (phys_base == NULL)
		return;

	free_ppages (phys_base, count);

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

/*!
 * Initializes the memory management subsystem.
 * Sets the base pointer for the PML4 table and stores the HHDM offset.
 * @param p_hhdm_offset The higher half direct mapping offset.
 */
void __init_memmgt__ (uint64_t p_hhdm_offset, struct limine_memmap_response* memmap_response) {
	idt_register_handler (0xE, (irq_handler_t)page_fault_handler);
	memmap_response_ptr = memmap_response;

	uint64_t pml4_base = read_cr3 () & 0xFFFFFFFFFF000;
	pml4_base_ptr = (pml4t_entry_t*)(pml4_base + p_hhdm_offset);
	hhdm_offset = p_hhdm_offset;

	// set up bitmap for physical page allocation
	init_physical_bitmap (memmap_response);

	paddr_t pdpt_frame = alloc_ppage ();
	memset (get_vaddr_from_frame ((uint64_t)pdpt_frame / PAGE_SIZE), 0, PAGE_SIZE);

	pml4_base_ptr[1].present = 1;
	pml4_base_ptr[1].read_write = 1;
	pml4_base_ptr[1].user_supervisor = 1;
	pml4_base_ptr[1].pdpt_base_address = ((uint64_t)pdpt_frame) / PAGE_SIZE;
}

/*!
 * Walks the page table hierarchy and prints present entries and their address ranges.
 */
void walk_pagetable () {
	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[1];
	if (!pml4t_entry->present)
		return;

	pdpt_entry_t* pdpt_base_ptr =
		(pdpt_entry_t*)get_vaddr_from_frame (pml4t_entry->pdpt_base_address);
	pdpt_entry_t* pdpt_entry = &pdpt_base_ptr[0];
	if (!pdpt_entry->present)
		return;

	pd_entry_t* pd_base_ptr = (pd_entry_t*)get_vaddr_from_frame (pdpt_entry->pd_base_address);
	for (int k = 0; k < 512; k++) {
		pd_entry_t* pd_entry = &pd_base_ptr[k];
		if (!pd_entry->present)
			continue;
		printf ("PD %d: PT Base Address:   0x%lx\n", k, pd_entry->pt_base_address << 12);
		pt_entry_t* pt_base_ptr = (pt_entry_t*)get_vaddr_from_frame (pd_entry->pt_base_address);

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

	pdpt_entry_t* pdpt_base_ptr =
		(pdpt_entry_t*)get_vaddr_from_frame (pml4t_entry->pdpt_base_address);
	pdpt_entry_t* pdpt_entry = &pdpt_base_ptr[virtual_addr.pdpt_index];
	if (!pdpt_entry->present)
		return NULL;

	if (pdpt_entry->page_size) {
		uint64_t phys_addr = (pdpt_entry->pd_base_address << 12) |
							 ((uint64_t)virtual_addr.pd_index << 21) |
							 ((uint64_t)virtual_addr.pt_index << 12) | virtual_addr.offset;
		return (void*)phys_addr;
	}

	pd_entry_t* pd_base_ptr = (pd_entry_t*)get_vaddr_from_frame (pdpt_entry->pd_base_address);
	pd_entry_t* pd_entry = &pd_base_ptr[virtual_addr.pd_index];
	if (!pd_entry->present)
		return NULL;

	if (pd_entry->page_size) {
		uint64_t phys_addr = (pd_entry->pt_base_address << 12) |
							 ((uint64_t)virtual_addr.pt_index << 12) | virtual_addr.offset;
		return (void*)phys_addr;
	}

	pt_entry_t* pt_base_ptr = (pt_entry_t*)get_vaddr_from_frame (pd_entry->pt_base_address);
	pt_entry_t* pt_entry = &pt_base_ptr[virtual_addr.pt_index];
	if (!pt_entry->present)
		return NULL;

	uint64_t phys_addr = (pt_entry->frame_base_address << 12) | virtual_addr.offset;

	return (void*)phys_addr;
}

/*

LIBALLOC FUNCTION IMPLEMENTATIONS

*/

int liballoc_lock () {
	is_locked = true;
	return 0;
}

int liballoc_unlock () {
	is_locked = false;
	return 0;
}

void* liballoc_alloc (size_t count) { return alloc_vpages (count, false); }

int liballoc_free (void* ptr, size_t count) {
	if (is_locked)
		return -7;

	free_vpages (ptr, count);
	return 0;
}