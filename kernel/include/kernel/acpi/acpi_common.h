#ifndef ACPI_SDT_HEADER_H
#define ACPI_SDT_HEADER_H

#include <stdint.h>

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

SDT_header_t* acpi_allocate_table (uint32_t phys_address);

bool	 acpi_validate_checksum (SDT_header_t* header);
uint64_t acpi_data_length (SDT_header_t* header);

#endif
