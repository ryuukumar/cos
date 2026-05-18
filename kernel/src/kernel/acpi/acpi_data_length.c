/*
 * acpi_data_length.c
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
 * Returns the number of bytes occupied by the ACPI table, excepting the bytes occupied by its
 * common header. Behavior undefined if header points to unallocated memory.
 * @param header the allocated SDT_header_t (typically returned from acpi_allocate_table)
 * @return size of ACPI data in bytes
 */
uint64_t acpi_data_length (SDT_header_t* header) { return header->length - sizeof (SDT_header_t); }