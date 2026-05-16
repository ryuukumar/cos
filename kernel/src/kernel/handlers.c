#include <kclib/stdio.h>
#include <kernel/handlers.h>
#include <kernel/idt.h>
#include <kernel/process.h>
#include <kernel/signal.h>

void handle_gpf (registers_t* registers) {
	kserial_printf ("Triggered GPF on PID #%lld. Sending SIGILL.\n", get_current_process ()->p_id);
	log_registers_to_serial (registers);
	send_signal (get_current_process (), SIGILL);
	deliver_pending_signals (registers);
}

void init_handlers (void) { idt_register_handler (0xD, handle_gpf); }