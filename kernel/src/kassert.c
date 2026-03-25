#include <kassert.h>
#include <kernel/serial.h>

/*!
 * Assert condition is true, if not, then busy wait
 * @param condition condition to be checked
 */
inline bool kassert_halt (bool condition) {
	if (condition) return true;
	for (;;)
		;
}

/*!
 * Assert condition is true, if not, then forward to handler
 * @param condition condition to be checked
 * @param message message to be forwarded to handler
 * @param handler handler to forward to
 */
inline bool kassert_handle (bool condition, const char* message, kassert_handler handler) {
	if (condition) return true;
	return handler (condition, message);
}

/*!
 * Assert condition is true, if not, then print to serial and busy wait
 * @param condition condition to be checked
 * @param message message to be printed
 */
inline bool kassert_print_halt (bool condition, const char* message) {
	if (condition) return true;
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
inline bool kassert_print_no_block (bool condition, const char* message) {
	if (condition) return true;
	write_serial_str ("KASSERT (non-block) failed for:\n");
	write_serial_str (message);
	return false;
}

/*!
 * Assert condition is true, if not, then print to serial and forward to handler
 * @param condition condition to be checked
 * @param message message to be printed and forwarded to handler
 * @param handler handler to forward to
 */
inline bool kassert_print_handle (bool condition, const char* message, kassert_handler handler) {
	if (condition) return true;
	write_serial_str ("KASSERT (handle) failed for:\n");
	write_serial_str (message);
	return handler (condition, message);
}