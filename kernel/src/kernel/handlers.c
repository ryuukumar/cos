#include <kclib/stdio.h>
#include <kernel/handlers.h>
#include <kernel/idt.h>
#include <kernel/process.h>

void handle_gpf (registers_t* registers) {
	kserial_printf ("Triggered GPF on PID #%lld\n", get_current_process ()->p_id);
	log_registers_to_serial (registers);
	for (;;)
		;
}

void init_handlers (void) { idt_register_handler (0xD, handle_gpf); }