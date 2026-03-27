#include <kclib/stdio.h>
#include <kernel/handlers.h>

registers_t* handle_gpf (registers_t* registers) {
	kserial_printf ("\nOopsy! Looks like someone tried to execute a disallowed instruction!");
	for (;;)
		;
	return registers; // dead code but shuts up unused var warning
}

void init_handlers (void) { idt_register_handler (0xD, handle_gpf); }