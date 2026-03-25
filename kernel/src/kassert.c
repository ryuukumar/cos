#include <kassert.h>
#include <kernel/serial.h>

/*!
 * Assert condition is true, if not, then busy wait
 * @param condition condition to be checked
 */
inline void kassert_halt (bool condition) {
	if (condition) return;
	for (;;)
		;
}

/*!
 * Assert condition is true, if not, then forward to handler
 * @param condition condition to be checked
 * @param message message to be forwarded to handler
 * @param handler handler to forward to
 */
inline void kassert_handle (bool condition, const char* message, kassert_handler handler) {
	if (condition) return;
	handler (condition, message);
}

/*!
 * Assert condition is true, if not, then print to serial and busy wait
 * @param condition condition to be checked
 * @param message message to be printed
 */
inline void kassert_print_halt (bool condition, const char* message) {
	if (condition) return;
	write_serial_str ("KASSERT (halt) failed for:\n");
	write_serial_str (message);
	for (;;)
		;
}

/*!
 * Assert condition is true, if not, then print to serial and continue execution
 * @param condition condition to be checked
 * @param message message to be printed
 */
inline void kassert_print_no_block (bool condition, const char* message) {
	if (condition) return;
	write_serial_str ("KASSERT (non-block) failed for:\n");
	write_serial_str (message);
}

/*!
 * Assert condition is true, if not, then print to serial and forward to handler
 * @param condition condition to be checked
 * @param message message to be printed and forwarded to handler
 * @param handler handler to forward to
 */
inline void kassert_print_handle (bool condition, const char* message, kassert_handler handler) {
	if (condition) return;
	write_serial_str ("KASSERT (handle) failed for:\n");
	write_serial_str (message);
	handler (condition, message);
}