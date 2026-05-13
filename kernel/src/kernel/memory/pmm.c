#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/memmgt.h>
#include <kernel/memory/pmm.h>

extern struct limine_memmap_response* memmap_response_ptr;
static memmap_bitmap				  bitmap;

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
paddr_t alloc_ppage (void) { return alloc_ppages (1); }

/*!
 * Free multiple consecutive physical frames
 * @param paddr base physical address of frames to free
 * @param count number of frames to free
 */
void free_ppages (void* paddr, uint64_t count) {
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
void free_ppage (void* paddr) { free_ppages (paddr, 1); }

void init_physical_bitmap (struct limine_memmap_response* memmap_response) {
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

	bitmap.map = (uint8_t*)((uint64_t)bitmap_phys_addr + get_hhdm_offset ());
	bitmap.pages_base = 0;
	bitmap.pages_maxlen = total_pages;
	bitmap.pages_used = total_pages;

	kmemset (bitmap.map, 0xFF, bitmap_size);

	uint64_t bitmap_fst_page = (uint64_t)bitmap_phys_addr / PAGE_SIZE;
	uint64_t bitmap_lst_page = bitmap_fst_page + ((bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE);

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			uint64_t start_page = (entry->base + PAGE_SIZE - 1) / PAGE_SIZE;
			uint64_t end_page = (entry->base + entry->length) / PAGE_SIZE;
			for (uint64_t p = start_page; p < end_page; p++)
				if (p < bitmap_fst_page || p >= bitmap_lst_page) bitmap_clear_bit (p);
		}
	}
}
