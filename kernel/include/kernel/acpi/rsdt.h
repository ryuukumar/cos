#ifndef ACPI_RSDT_H
#define ACPI_RSDT_H

#include <kernel/acpi/acpi_common.h>
#include <kernel/acpi/rsdp.h>

typedef struct __attribute__ ((packed)) {
	SDT_header_t header;
	uint32_t	 other_sdt_ptrs[];
} RSDT_t;

void init_rsdt (uint32_t rsdt_base_ptr, uint64_t hhdm_offset);

#endif