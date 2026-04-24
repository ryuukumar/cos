#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/acpi/fadt.h>
#include <kernel/acpi/madt.h>
#include <kernel/acpi/rsdt.h>

static void parse_by_table (uint32_t phys_address) {
	SDT_header_t* table_header = acpi_allocate_table (phys_address);
	if (!acpi_validate_checksum (table_header)) return;

	char buffer[sizeof (((SDT_header_t*)0)->signature) + 1] = {0};
	kmemcpy (&buffer[0], &table_header->signature, sizeof (buffer) - 1);

	if (kstrncmp (&buffer[0], FADT_IDENTIFIER, 4) == 0)
		init_fadt (table_header);
	else if (kstrncmp (&buffer[0], MADT_IDENTIFIER, 4) == 0)
		init_madt (table_header);
	else
		kserial_printf ("[ACPI] Unrecognised table signature: %s\n", &buffer);
}

void init_rsdt (uint32_t rsdt_base_ptr) {
	SDT_header_t* rsdt_ptr = acpi_allocate_table (rsdt_base_ptr);
	if (!acpi_validate_checksum (rsdt_ptr)) return;

	uint32_t* data_first_address = (uint32_t*)((char*)rsdt_ptr + sizeof (SDT_header_t));
	for (uint64_t i = 0; sizeof (uint32_t) * i < acpi_data_length (rsdt_ptr); i++)
		parse_by_table (data_first_address[i]);
}