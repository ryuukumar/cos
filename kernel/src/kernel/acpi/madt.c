#include <kclib/string.h>
#include <kernel/acpi/madt.h>
#include <liballoc/liballoc.h>

static MADT_header_t* cp_madt = nullptr;

void init_madt (SDT_header_t* header) {
	uint64_t madt_len = header->length;
	cp_madt = kmalloc (madt_len);
	if (!cp_madt) return;

	kmemcpy ((void*)cp_madt, (void*)header, madt_len);
}

MADT_header_t* get_madt_header (void) { return cp_madt; }

MADT_entry_t* get_nth_entry (size_t n) {
	if (!cp_madt) return nullptr;
	MADT_entry_t* entry = (MADT_entry_t*)&cp_madt[1];

	for (size_t i = 0; (char*)entry - (char*)cp_madt < cp_madt->header.length; i++) {
		if (i == n) return entry;
		if (entry->header_only.record_length == 0) return nullptr;
		entry = (MADT_entry_t*)(((char*)entry) + entry->header_only.record_length);
	}

	return nullptr;
}