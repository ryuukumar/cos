#include <kernel/acpi/acpi_common.h>
#include <kernel/memmgt.h>

/*!
 * Allocate all the memory occupied by the ACPI table at the given physical address.
 * @param phys_address the physical address of the ACPI table
 * @return pointer to the first byte of the ACPI table in virtual memory as SDT_header_t
 */
SDT_header_t* acpi_allocate_table (uint32_t phys_address) {
	// stage 1: allocate just enough to read the header
	uint64_t table_base_ptr_64 = (uint64_t)phys_address;
	uint64_t hhdm_offset = get_hhdm_offset ();
	paddr_t	 table_base_frame = (paddr_t)ALIGN_PAGE_DOWN (table_base_ptr_64);
	table_base_ptr_64 += hhdm_offset;

	vaddr_t table_vaddr_start = get_vaddr_t_from_ptr ((void*)ALIGN_PAGE_DOWN (table_base_ptr_64));
	vaddr_t table_vaddr_end =
		get_vaddr_t_from_ptr ((void*)ALIGN_PAGE_DOWN (table_base_ptr_64 + sizeof (SDT_header_t)));
	alloc_all_vpages_in_range (table_vaddr_start, table_vaddr_end, table_base_frame);

	// stage 2: allocate enough to read the entire table
	SDT_header_t* table_ptr = (SDT_header_t*)table_base_ptr_64;
	table_vaddr_end =
		get_vaddr_t_from_ptr ((void*)ALIGN_PAGE_DOWN (table_base_ptr_64 + table_ptr->length));
	alloc_all_vpages_in_range (table_vaddr_start, table_vaddr_end, table_base_frame);

	return table_ptr;
}