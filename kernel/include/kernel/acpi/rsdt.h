#ifndef ACPI_RSDT_H
#define ACPI_RSDT_H

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

#endif
