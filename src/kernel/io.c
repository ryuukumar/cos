
#include <kernel/io.h>


/*!
 * Write a single byte to a port
 * @param port port to write
 * @param val value to write
 */
static inline void outb (uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

/*!
 * Read a single byte from a port
 * @param port port to read
 * @return value read from port
 */
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

