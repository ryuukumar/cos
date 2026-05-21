/*
 * acpi_allocate_table.c
 * Copyright (C) 2026  Aditya Kumar
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, see <https://www.gnu.org/licenses/>.
 */

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
	alloc_all_vpages_in_range (table_vaddr_start, table_vaddr_end, table_base_frame, M_PG_READ);

	// stage 2: allocate enough to read the entire table
	SDT_header_t* table_ptr = (SDT_header_t*)table_base_ptr_64;
	table_vaddr_end =
		get_vaddr_t_from_ptr ((void*)ALIGN_PAGE_DOWN (table_base_ptr_64 + table_ptr->length));
	alloc_all_vpages_in_range (table_vaddr_start, table_vaddr_end, table_base_frame, M_PG_READ);

	return table_ptr;
}