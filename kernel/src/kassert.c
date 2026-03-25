#include <kassert.h>
#include <kernel/serial.h>

constexpr char empty_str[] = "";

inline void kassert_halt (bool condition) {
	if (condition) return;
	for (;;)
		;
}

inline void kassert_handle (bool condition, kassert_handler handler) {
	if (condition) return;
	handler (condition, (const char*)&empty_str);
}

inline void kassert_print_halt (bool condition, const char* message) {
	if (condition) return;
	write_serial_str ("KASSERT (halt) failed for:\n");
	write_serial_str (message);
	for (;;)
		;
}

inline void kassert_print_no_block (bool condition, const char* message) {
	if (condition) return;
	write_serial_str ("KASSERT (non-block) failed for:\n");
	write_serial_str (message);
}

inline void kassert_print_handle (bool condition, const char* message, kassert_handler handler) {
	if (condition) return;
	write_serial_str ("KASSERT (handle) failed for:\n");
	write_serial_str (message);
	handler (condition, message);
}