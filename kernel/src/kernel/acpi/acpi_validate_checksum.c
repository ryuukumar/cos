#include <kernel/acpi/acpi_common.h>

/*!
 * Validates the ACPI table with the present checksum. Behavior undefined if header points to
 * unallocated memory.
 * @param header the allocated SDT_header_t (typically returned from acpi_allocate_table)
 * @return true if ACPI table checksum is valid, false otherwise
 */
bool acpi_validate_checksum (SDT_header_t* header) {
	uint8_t checksum = 0;
	for (uint64_t i = 0; i < sizeof (SDT_header_t); i++)
		checksum += ((uint8_t*)header)[i];
	return checksum == 0;
}