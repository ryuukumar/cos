/*
 * acpi_common.h
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

#pragma once

#include <stdint.h>

#define FADT_IDENTIFIER "FACP"
#define MADT_IDENTIFIER "APIC"

typedef struct __attribute__ ((packed)) {
	char	 signature[4];
	uint32_t length;
	uint8_t	 revision;
	uint8_t	 checksum;
	char	 oemid[6];
	char	 oemTableId[8];
	uint32_t oemRevision;
	uint32_t creatorId;
	uint32_t creatorRevision;
} SDT_header_t;

typedef struct __attribute__ ((packed)) {
	uint8_t	 address_space;
	uint8_t	 bit_width;
	uint8_t	 bit_offset;
	uint8_t	 access_size;
	uint64_t address;
} ACPI_GAS_t;

SDT_header_t* acpi_allocate_table (uint32_t phys_address);

bool	 acpi_validate_checksum (SDT_header_t* header);
uint64_t acpi_data_length (SDT_header_t* header);
