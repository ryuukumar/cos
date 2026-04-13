#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/acpi/rsdt.h>
#include <kernel/memmgt.h>

static void parse_by_table (uint32_t phys_address) {
	SDT_header_t* table_header = acpi_allocate_table (phys_address);
	if (!acpi_validate_checksum (table_header)) return;

	char buffer[sizeof (((SDT_header_t*)0)->signature) + 1] = {0};
	kmemcpy (&buffer[0], &table_header->signature, sizeof (buffer) - 1);

	// Right now nothing is recognised, so just fall through to this message
	kserial_printf ("[ACPI] Unrecognised table signature: %s\n", &buffer);
}

void init_rsdt (uint32_t rsdt_base_ptr) {
	SDT_header_t* rsdt_ptr = acpi_allocate_table (rsdt_base_ptr);
	uint32_t*	  data_first_address = (uint32_t*)((char*)rsdt_ptr + sizeof (SDT_header_t));

	for (uint64_t i = 0; sizeof (uint32_t) * i < acpi_data_length (rsdt_ptr); i++)
		parse_by_table (data_first_address[i]);
}