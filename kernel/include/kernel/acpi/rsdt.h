#ifndef ACPI_RSDT_H
#define ACPI_RSDT_H

#include <kernel/acpi/sdt_header.h>

typedef struct __attribute__ ((packed)) {
	SDT_header_t header;
	uint32_t	 other_sdt_ptrs[];
} RSDT_t;

void init_rsdt (RSDP_t* rsdp_base_ptr);

#endif