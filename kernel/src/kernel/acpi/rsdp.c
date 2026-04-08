#include <kernel/acpi/rsdp.h>
#include <kernel/memmgt.h>

static RSDP_t* rsdp_ptr = nullptr;
static XSDP_t* xsdp_ptr = nullptr;
static uint8_t rsdp_ver = 0;

uint8_t get_rsdp_revision () { return rsdp_ver; }

void* init_rsdp (uintptr_t rsdp_base_ptr, uint64_t hhdm_offset) {
	if (rsdp_base_ptr == 0) return nullptr;

	// check if address is physical ; if it is, we will probably have to allocate memory
	if (rsdp_base_ptr < hhdm_offset) {
		paddr_t rsdp_base_frame = (paddr_t)ALIGN_PAGE_DOWN (rsdp_base_ptr);
		vaddr_t rsdp_vaddr_start =
			get_vaddr_t_from_ptr ((void*)ALIGN_PAGE_DOWN (rsdp_base_ptr + hhdm_offset));
		vaddr_t rsdp_vaddr_end = get_vaddr_t_from_ptr (
			(void*)ALIGN_PAGE_DOWN (rsdp_base_ptr + hhdm_offset + sizeof (XSDP_t)));

		alloc_all_vpages_in_range (rsdp_vaddr_start, rsdp_vaddr_end, rsdp_base_frame);
	}

	RSDP_t* tmp_rsdp_ptr = (RSDP_t*)rsdp_base_ptr;
	XSDP_t* tmp_xsdp_ptr = (XSDP_t*)rsdp_base_ptr;

	// validate the RSDP & set revision
	uint8_t rsdp_checksum = 0;
	for (uint64_t i = 0; i < sizeof (RSDP_t); i++)
		rsdp_checksum += ((uint8_t*)tmp_rsdp_ptr)[i];
	if (rsdp_checksum != 0) return nullptr;
	rsdp_ver = tmp_rsdp_ptr->revision;

	// if we are on rev 1, set rsdp_ptr and leave
	if (rsdp_ver == 1) return rsdp_ptr = tmp_rsdp_ptr;

	// validate the XSDP
	uint8_t xsdp_checksum = 0;
	for (uint64_t j = sizeof (RSDP_t); j < sizeof (XSDP_t); j++)
		xsdp_checksum += ((uint8_t*)tmp_xsdp_ptr)[j];
	if (xsdp_checksum != 0) return nullptr;

	return xsdp_ptr = tmp_xsdp_ptr;
}