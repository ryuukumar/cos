#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/idt.h>
#include <kernel/limine.h>
#include <kernel/memmgt.h>
#include <kernel/memory/pmm.h>
#include <kernel/process.h>
#include <kernel/syscall.h>

#define ALIGNUP(x, o) ((x + o - 1) & ~(o - 1))

static pml4t_entry_t* pml4_base_ptr = nullptr;
static uint64_t		  hhdm_offset = 0;
static uintptr_t	  kernel_cr3 = 0;

struct limine_memmap_response* memmap_response_ptr;

/*!
 * Get the CR3 for the kernel
 * @return kernel CR3
 */
uintptr_t get_kernel_cr3 (void) { return kernel_cr3; }

/*!
 * Get the HHDM offset
 * @return HHDM offset
 */
uint64_t get_hhdm_offset (void) { return hhdm_offset; }

/*!
 * Get the PML4 base pointer
 * @return PML4 base pointer
 */
pml4t_entry_t* get_pml4_baseptr (void) { return pml4_base_ptr; }

/*!
 * Simple handler for page fault, prints faulting address from CR2
 */
static void page_fault_handler (registers_t* registers) {
	uint64_t cr2;
	__asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));

	kserial_printf ("\nEncountered a page fault!\nFaulting address: 0x%llx\n", cr2);
	log_registers_to_serial (registers);

	for (;;)
		;
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
	init_physical_bitmap (memmap_response);

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

	init_vmm (pml4_base_ptr);

	register_syscall (SYSCALL_SYS_BRK, sys_brk);
}

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
