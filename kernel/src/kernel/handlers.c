#include <kernel/handlers.h>
#include <kernel/serial.h>

registers_t* handle_gpf (registers_t* registers) {
	write_serial_str ("\nOopsy! Looks like someone tried to execute a disallowed instruction!");
	for (;;)
		;
	return registers; // dead code but shuts up unused var warning
}

void __init_handlers__ (void) { idt_register_handler (0xD, handle_gpf); }