
#include <kernel/io.h>

/*!
 * Write a single byte to a port
 * @param port port to write
 * @param val value to write
 */
inline void outb (uint16_t port, uint8_t val) { __asm__ volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory"); }

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