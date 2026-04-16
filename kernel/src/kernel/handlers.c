#include <kclib/stdio.h>
#include <kernel/handlers.h>

static bool is_init_gpf_b = false;

void handle_gpf (registers_t* registers) {
	(void)registers;
	kserial_printf ("\nOopsy! Looks like someone tried to execute a disallowed instruction!");
	for (;;)
		;
}

void init_handlers (void) {
	idt_register_handler (0xD, handle_gpf);
	is_init_gpf_b = true;
}

bool is_init_gpf (void) { return is_init_gpf_b; }
