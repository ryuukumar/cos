#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/idt.h>
#include <kernel/limine.h>
#include <kernel/memmgt.h>
#include <kernel/process.h>
#include <kernel/syscall.h>

#define ALIGNUP(x, o) ((x + o - 1) & ~(o - 1))

#define USER_PML4_IDX 1
#define KRNL_PML4_IDX 257

pml4t_entry_t* pml4_base_ptr = nullptr;
uint64_t	   hhdm_offset = 0;

bool is_locked = false;

memmap_bitmap bitmap;

struct limine_memmap_response* memmap_response_ptr;

uintptr_t kernel_cr3 = 0;

/*!
 * Reads the value of the CR3 register, which contains the physical address of the PML4 table.
 * @return The value of the CR3 register.
 */
inline uint64_t read_cr3 (void) {
	uint64_t cr3;
	__asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
	return cr3;
}

/*!
 * Writes a new value to the CR3 register.
 * @param new_value the new value of CR3
 */
inline void write_cr3 (uint64_t new_value) {
	__asm__ volatile ("mov %0, %%cr3" : : "r"(new_value) : "memory");
}

/*!
 * Get a vaddr_t object from a virtual pointer
 * @param ptr the virtual pointer
 * @return corresponding vaddr_t object
 */
vaddr_t get_vaddr_t_from_ptr (void* ptr) {
	uint64_t ptr_64t = (uint64_t)ptr;
	vaddr_t	 ret_vaddr;

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

	if (ptr_64t & (1ULL << 47)) ptr_64t |= 0xFFFF000000000000ULL;
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
static paddr_t alloc_ppages (uint64_t count) {
	if (count == 0) return nullptr;
	if (bitmap.pages_used + count > bitmap.pages_maxlen) return nullptr;

	uint64_t current_streak = 0;
	uint64_t start_idx = 0;

	for (uint64_t i = 0; i < bitmap.pages_maxlen; i++) {
		if (i % 8 == 0 && current_streak == 0 && bitmap.map[i / 8] == 0xFF) {
			i += 7;
			continue;
		}

		if ((bitmap.map[i / 8] & (1 << (i % 8))) == 0) {
			if (current_streak == 0) start_idx = i;
			current_streak++;

			if (current_streak == count) {
				for (uint64_t j = start_idx; j < start_idx + count; j++)
					bitmap_set_bit (j);
				return (paddr_t)(bitmap.pages_base + PAGE_SIZE * start_idx);
			}
		} else
			current_streak = 0;
	}

	return nullptr;
}

/*!
 * Allocate a single physical frame
 * @return base physical address of allocated frame
 */
static paddr_t alloc_ppage (void) { return alloc_ppages (1); }

/*!
 * Free multiple consecutive physical frames
 * @param paddr base physical address of frames to free
 * @param count number of frames to free
 */
static void free_ppages (void* paddr, uint64_t count) {
	uint64_t start_idx = (uint64_t)paddr / PAGE_SIZE;

	for (uint64_t i = 0; i < count; i++) {
		uint64_t current_paddr = (uint64_t)paddr + (i * PAGE_SIZE);
		bool	 is_valid = false;

		if (memmap_response_ptr != nullptr) {
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
			kserial_printf ("Tried to free reserved memory! Address: 0x%llx\nHalting.\n",
							(uint64_t)paddr);
			__asm__ ("hlt");
		}

		bitmap_clear_bit (start_idx + i);
	}
}

/*!
 * Free a single physical frame
 * @param paddr base physical address of frame to free
 */
static void free_ppage (void* paddr) { free_ppages (paddr, 1); }

static uint64_t init_physical_bitmap (struct limine_memmap_response* memmap_response) {
	uint64_t addr_limit = 0;

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			uint64_t top = entry->base + entry->length;
			if (top > addr_limit) addr_limit = top;
		}
	}

	uint64_t total_pages = addr_limit / PAGE_SIZE;
	uint64_t bitmap_size = (total_pages + 7) / 8;

	void* bitmap_phys_addr = nullptr;
	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
			bitmap_phys_addr = (void*)entry->base;
			break;
		}
	}

	if (bitmap_phys_addr == nullptr) {
		kserial_printf (
			"Not enough contiguous memory for bitmap setup!! Please download some RAM.\n");
		kprintf ("Not enough contiguous memory for bitmap setup!! Please download some RAM.\n");
		__asm__ ("hlt");
	}

	bitmap.map = (uint8_t*)((uint64_t)bitmap_phys_addr + hhdm_offset);
	bitmap.pages_base = 0;
	bitmap.pages_maxlen = total_pages;
	bitmap.pages_used = total_pages;

	kmemset (bitmap.map, 0xFF, bitmap_size);

	uint64_t bitmap_fst_page = (uint64_t)bitmap_phys_addr / PAGE_SIZE;
	uint64_t bitmap_lst_page = bitmap_fst_page + ((bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE);

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			for (uint64_t p = entry->base / PAGE_SIZE;
				 p < (entry->base + entry->length) / PAGE_SIZE; p++) {
				if (p < bitmap_fst_page || p >= bitmap_lst_page) bitmap_clear_bit (p);
			}
		}
	}

	return addr_limit;
}

/*!
 * Simple handler for page fault, prints faulting address from CR2
 */
static void page_fault_handler (registers_t* registers) {
	(void)registers;

	uint64_t cr2;
	__asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));

	kserial_printf ("\nEncountered a page fault!\nFaulting address: 0x%llx\n", cr2);

	for (;;)
		;
}

/*!
 * Check if vaddr is supposed to be a user page. Practically just a check if pml4 index is at least
 * 256.
 * @param addr vaddr to check
 * @return whether it is a user page
 */
static inline bool is_vaddr_t_user (vaddr_t* addr) { return addr->pml4_index < 256; }

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
				if (a->pt_index == b->pt_index) return a->offset < b->offset;
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
	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[first.pml4_index];
	if (!pml4t_entry->present) return;

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
 * Free multiple consecutive virtual pages=
 * @param ptr virtual address of the first page to free
 * @param count number of consecutive pages to free
 */
void free_vpages (void* ptr, size_t count) {
	if (ptr == nullptr || count == 0) return;

	vaddr_t vaddr = get_vaddr_t_from_ptr (ptr);
	if (vaddr.pml4_index != 1) {
		kserial_printf ("free_vpages called for pml4_index that is not 1!");
		return;
	}

	void* phys_base = get_paddr (ptr);
	if (phys_base == nullptr) return;

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

void alloc_by_cr3 (uint64_t cr3, uintptr_t start, size_t num_pages, bool write) {
	pml4t_entry_t* original_ptr = pml4_base_ptr;
	pml4_base_ptr = (pml4t_entry_t*)(cr3 + hhdm_offset);

	paddr_t physmem = alloc_ppages (num_pages);
	alloc_all_vpages_in_range (get_vaddr_t_from_ptr ((void*)start),
							   get_vaddr_t_from_ptr ((void*)(start + (num_pages * PAGE_SIZE))),
							   physmem);

	(void)write; // TODO: set rw flag

	pml4_base_ptr = original_ptr;
}

void dealloc_by_cr3 (uint64_t cr3, uintptr_t start, size_t num_pages) {
	pml4t_entry_t* original_ptr = pml4_base_ptr;
	pml4_base_ptr = (pml4t_entry_t*)(cr3 + hhdm_offset);

	free_all_vpages_in_range (get_vaddr_t_from_ptr ((void*)start),
							  get_vaddr_t_from_ptr ((void*)(start + (num_pages * PAGE_SIZE))));

	pml4_base_ptr = original_ptr;
}

static uint64_t sys_brk (uint64_t addr, uint64_t arg2, uint64_t arg3) {
	(void)arg2, (void)arg3;

	process* current = get_current_process ();
	if (!current) return 0;
	if (addr < current->p_heap_base) return current->p_heap_base + current->p_heap_sz;

	size_t old_pcount = ALIGNUP (current->p_heap_sz, PAGE_SIZE) / PAGE_SIZE;
	size_t new_pcount = ALIGNUP ((uintptr_t)addr - current->p_heap_base, PAGE_SIZE) / PAGE_SIZE;

	if (new_pcount > old_pcount)
		alloc_by_cr3 (current->p_cr3, current->p_heap_base + PAGE_SIZE * old_pcount,
					  new_pcount - old_pcount, true);

	if (new_pcount < old_pcount)
		dealloc_by_cr3 (current->p_cr3, current->p_heap_base + PAGE_SIZE * new_pcount,
						old_pcount - new_pcount);

	current->p_heap_sz = addr - current->p_heap_base;
	return current->p_heap_base + current->p_heap_sz;
}

/*!
 * Initializes the HHDM at hhdm_offset.
 * @param memsz The size of memory available.
 * @param hhdm_offset The higher half direct mapping offset.
 */
void init_hhdm (uint64_t memsz, uint64_t hhdm_offset) {
	if (memsz == 0) return;
	vaddr_t start = get_vaddr_t_from_ptr ((void*)hhdm_offset);
	vaddr_t end = get_vaddr_t_from_ptr ((void*)(hhdm_offset + memsz - 1));
	alloc_all_vpages_in_range(start, end, 0);
}

/*!
 * Initializes the memory management subsystem.
 * Sets the base pointer for the PML4 table and stores the HHDM offset.
 * @param p_hhdm_offset The higher half direct mapping offset.
 */
void init_memmgt (uint64_t p_hhdm_offset, struct limine_memmap_response* memmap_response) {
	idt_register_handler (0xE, (irq_handler_t)page_fault_handler);
	memmap_response_ptr = memmap_response;

	uint64_t pml4_base = read_cr3 () & 0xFFFFFFFFFF000;
	pml4_base_ptr = (pml4t_entry_t*)(pml4_base + p_hhdm_offset);
	hhdm_offset = p_hhdm_offset;
	kernel_cr3 = read_cr3 ();

	// set up bitmap for physical page allocation
	uint64_t addr_size = init_physical_bitmap (memmap_response);

	paddr_t user_pdpt_frame = alloc_ppage ();
	kmemset (get_vaddr_from_frame ((uint64_t)user_pdpt_frame / PAGE_SIZE), 0, PAGE_SIZE);

	pml4_base_ptr[USER_PML4_IDX].present = 1;
	pml4_base_ptr[USER_PML4_IDX].read_write = 1;
	pml4_base_ptr[USER_PML4_IDX].user_supervisor = 1;
	pml4_base_ptr[USER_PML4_IDX].pdpt_base_address = ((uint64_t)user_pdpt_frame) / PAGE_SIZE;

	paddr_t krnl_pdpt_frame = alloc_ppage ();
	kmemset (get_vaddr_from_frame ((uint64_t)krnl_pdpt_frame / PAGE_SIZE), 0, PAGE_SIZE);

	pml4_base_ptr[KRNL_PML4_IDX].present = 1;
	pml4_base_ptr[KRNL_PML4_IDX].read_write = 1;
	pml4_base_ptr[KRNL_PML4_IDX].pdpt_base_address = ((uint64_t)krnl_pdpt_frame) / PAGE_SIZE;

	init_hhdm (addr_size, hhdm_offset);
	register_syscall (SYSCALL_SYS_BRK, sys_brk);
}

/*!
 * Get the CR3 for the kernel
 * @return kernel CR3
 */
uintptr_t get_kernel_cr3 (void) { return kernel_cr3; }

/*!
 * Finds the physical address mapped to a given virtual address.
 * @param vaddr The virtual address to look up.
 * @return The corresponding physical address, or nullptr if not mapped.
 */
void* get_paddr (void* vaddr) {
	vaddr_t virtual_addr = get_vaddr_t_from_ptr (vaddr);

	pml4t_entry_t* pml4t_entry = &pml4_base_ptr[virtual_addr.pml4_index];
	if (!pml4t_entry->present) return nullptr;

	pdpt_entry_t* pdpt_base_ptr =
		(pdpt_entry_t*)get_vaddr_from_frame (pml4t_entry->pdpt_base_address);
	pdpt_entry_t* pdpt_entry = &pdpt_base_ptr[virtual_addr.pdpt_index];
	if (!pdpt_entry->present) return nullptr;

	if (pdpt_entry->page_size) {
		uint64_t phys_addr = (pdpt_entry->pd_base_address << 12) |
							 ((uint64_t)virtual_addr.pd_index << 21) |
							 ((uint64_t)virtual_addr.pt_index << 12) | virtual_addr.offset;
		return (void*)phys_addr;
	}

	pd_entry_t* pd_base_ptr = (pd_entry_t*)get_vaddr_from_frame (pdpt_entry->pd_base_address);
	pd_entry_t* pd_entry = &pd_base_ptr[virtual_addr.pd_index];
	if (!pd_entry->present) return nullptr;

	if (pd_entry->page_size) {
		uint64_t phys_addr = (pd_entry->pt_base_address << 12) |
							 ((uint64_t)virtual_addr.pt_index << 12) | virtual_addr.offset;
		return (void*)phys_addr;
	}

	pt_entry_t* pt_base_ptr = (pt_entry_t*)get_vaddr_from_frame (pd_entry->pt_base_address);
	pt_entry_t* pt_entry = &pt_base_ptr[virtual_addr.pt_index];
	if (!pt_entry->present) return nullptr;

	uint64_t phys_addr = (pt_entry->frame_base_address << 12) | virtual_addr.offset;

	return (void*)phys_addr;
}

static paddr_t clone_pframes (paddr_t p_src, uint64_t page_count) {
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

/*

LIBALLOC FUNCTION IMPLEMENTATIONS

*/

int liballoc_lock (void) {
	is_locked = true;
	return 0;
}

int liballoc_unlock (void) {
	is_locked = false;
	return 0;
}

void* liballoc_alloc (size_t count) { return alloc_vpages (count, false); }

int liballoc_free (void* ptr, size_t count) {
	if (is_locked) return -7;

	free_vpages (ptr, count);
	return 0;
}