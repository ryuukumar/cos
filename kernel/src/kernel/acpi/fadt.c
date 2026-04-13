#include <kclib/string.h>
#include <kernel/acpi/acpi_common.h>
#include <kernel/acpi/fadt.h>
#include <liballoc/liballoc.h>

static FADT* cp_fadt = nullptr;

void init_fadt (SDT_header_t* header) {
	cp_fadt = kmalloc (sizeof (FADT));
	if (!cp_fadt) return;

	kmemcpy ((void*)cp_fadt, (void*)header, sizeof (FADT));
}

FADT* get_fadt_copy (void) {
    if (!cp_fadt) return nullptr;

	FADT* new_fadt = kmalloc (sizeof (FADT));
	if (!new_fadt) return nullptr;

	kmemcpy ((void*)new_fadt, (void*)cp_fadt, sizeof (FADT));
	return new_fadt;
}