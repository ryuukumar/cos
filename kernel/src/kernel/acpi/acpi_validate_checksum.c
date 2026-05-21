/*
 * acpi_validate_checksum.c
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

/*!
 * Validates the ACPI table with the present checksum. Behavior undefined if header points to
 * unallocated memory.
 * @param header the allocated SDT_header_t (typically returned from acpi_allocate_table)
 * @return true if ACPI table checksum is valid, false otherwise
 */
bool acpi_validate_checksum (SDT_header_t* header) {
	uint8_t checksum = 0;
	for (uint64_t i = 0; i < header->length; i++)
		checksum += ((uint8_t*)header)[i];
	return checksum == 0;
}