#include <kernel/memmgt.h>

/*!
 * Reads the value of the CR3 register, which contains the physical address of the PML4 table.
 * @return The value of the CR3 register.
 */
inline uint64_t read_cr3 (void) {
	uint64_t cr3;
	__asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
	return cr3;
}

/*!
 * Writes a new value to the CR3 register.
 * @param new_value the new value of CR3
 */
inline void write_cr3 (uint64_t new_value) {
	__asm__ volatile ("mov %0, %%cr3" : : "r"(new_value) : "memory");
}