
#include <kernel/io.h>

/*!
 * Write a single byte to a port
 * @param port port to write
 * @param val value to write
 */
inline void outb (uint16_t port, uint8_t val) {
	__asm__ volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

/*!
 * Read a single byte from a port
 * @param port port to read
 * @return value read from port
 */
inline uint8_t inb (uint16_t port) {
	uint8_t ret;
	__asm__ volatile ("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
	return ret;
}

/*!
 * Short wait (1-4 ms)
 */
inline void io_wait (void) { outb (0x80, 0); }

/*!
 * Get flags and clear interrupt flag.
 * @return flags before the interrupt flag was cleared
 */
inline uint64_t save_irq_disable (void) {
	uint64_t flags;
	__asm__ volatile ("pushfq\n\t"
					  "pop %0\n\t"
					  "cli"
					  : "=rm"(flags)
					  :
					  : "memory");
	return flags;
}

/*!
 * Restore flags.
 * @param flags flags to set (typically return value of save_irq_disable)
 */
inline void restore_irq (uint64_t flags) {
	__asm__ volatile ("push %0\n\t"
					  "popfq"
					  :
					  : "rm"(flags)
					  : "memory");
}