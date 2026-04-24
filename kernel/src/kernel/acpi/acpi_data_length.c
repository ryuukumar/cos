#include <kernel/acpi/acpi_common.h>

/*!
 * Returns the number of bytes occupied by the ACPI table, excepting the bytes occupied by its
 * common header. Behavior undefined if header points to unallocated memory.
 * @param header the allocated SDT_header_t (typically returned from acpi_allocate_table)
 * @return size of ACPI data in bytes
 */
uint64_t acpi_data_length (SDT_header_t* header) { return header->length - sizeof (SDT_header_t); }