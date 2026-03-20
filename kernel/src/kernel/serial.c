
#include <kernel/serial.h>

/*!
 * Initialise serial communications to port SERIAL_COM_1
 * Textbook implementation, see: https://wiki.osdev.org/Serial_Ports
 * @return 0 if initialised, 1 if port faulty
 */
int init_serial (void) {
	outb (SERIAL_COM_1 + 1, 0x00); // Disable all interrupts
	outb (SERIAL_COM_1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
	outb (SERIAL_COM_1 + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
	outb (SERIAL_COM_1 + 1, 0x00); //                  (hi byte)
	outb (SERIAL_COM_1 + 3, 0x03); // 8 bits, no parity, one stop bit
	outb (SERIAL_COM_1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
	outb (SERIAL_COM_1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
	outb (SERIAL_COM_1 + 4, 0x1E); // Set in loopback mode, test the serial chip
	outb (SERIAL_COM_1 + 0,
		  0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

	// Check if serial is faulty (i.e: not same byte as sent)
	if (inb (SERIAL_COM_1 + 0) != 0xAE)
		return 1;

	// If serial is not faulty set it in normal operation mode
	// (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
	outb (SERIAL_COM_1 + 4, 0x0F);
	return 0;
}

/*!
 * Read from SERIAL_COM_1
 * Textbook implementation, see: https://wiki.osdev.org/Serial_Ports
 * @return The value read from serial
 */
uint8_t read_serial (void) {
	while (!(inb (SERIAL_COM_1 + 5) & 1))
		;
	return inb (SERIAL_COM_1);
}

/*!
 * Write to SERIAL_COM_1
 * Textbook implementation, see: https://wiki.osdev.org/Serial_Ports
 * @param val The value to write to serial
 */
void write_serial (uint8_t val) {
	while (!(inb (SERIAL_COM_1 + 5) & 0x20))
		;
	outb (SERIAL_COM_1, val);
}

/*!
 * Write a string to SERIAL_COM_1
 * @param str The string to write to serial
 */
void write_serial_str (const char* str) {
	char* x = (char*)str;
	while (*x != 0)
		write_serial (*(x++));
}