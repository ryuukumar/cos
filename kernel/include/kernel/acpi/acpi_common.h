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
