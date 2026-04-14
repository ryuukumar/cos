#include <kernel/acpi/acpi.h>
#include <kernel/acpi/rsdp.h>
#include <kernel/acpi/rsdt.h>
#include <kernel/memmgt.h>

void init_acpi (uintptr_t rsdp_ptr) {
	RSDP_t* rsdp_ptr_vmm = (RSDP_t*)init_rsdp (rsdp_ptr, get_hhdm_offset ());
	init_rsdt (rsdp_ptr_vmm->rsdt_address);
}