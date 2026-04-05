
#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/idt.h>

__attribute__ ((aligned (0x10))) static idt_entry_t idt[256];

static idtr_t idtr;
irq_handler_t interrupt_handlers[256];
extern void*  isr_stub_table[];

// Declaration of handler for internal use only
void kernel_dispatch_interrupt (registers_t* registers);

static void idt_set_gate (uint8_t vector, void* isr, uint8_t gate_type, uint8_t dpl,
						  uint8_t present) {
	idt_entry_t* descriptor = &idt[vector];

	descriptor->offset_1 = (uint64_t)isr & 0xFFFF;
	descriptor->offset_2 = ((uint64_t)isr >> 16) & 0xFFFF;
	descriptor->offset_3 = ((uint64_t)isr >> 32) & 0xFFFFFFFF;

	descriptor->selector = 0x28;

	descriptor->ist = 0;
	descriptor->zero_1 = 0;
	descriptor->zero_2 = 0;
	descriptor->zero_3 = 0;

	descriptor->gate_type = gate_type & 0x0F;
	descriptor->dpl = dpl & 0x03;
	descriptor->present = present & 0x01;
}

static void log_registers_to_serial (registers_t* registers) {
	kserial_printf ("\n--- Interrupt/Exception Caught ---\n");

	kserial_printf ("interrupt_number: 0x%llx\n", registers->interrupt_number);
	kserial_printf ("error_code: 0x%llx\n", registers->error_code);
	kserial_printf ("rip: 0x%llx\n", registers->rip);
	kserial_printf ("cs: 0x%llx\n", registers->cs);
	kserial_printf ("rflags: 0x%llx\n", registers->rflags);
	kserial_printf ("rsp: 0x%llx\n", registers->rsp);
	kserial_printf ("ss: 0x%llx\n\n", registers->ss);

	kserial_printf ("rax: 0x%llx\n", registers->rax);
	kserial_printf ("rbx: 0x%llx\n", registers->rbx);
	kserial_printf ("rcx: 0x%llx\n", registers->rcx);
	kserial_printf ("rdx: 0x%llx\n", registers->rdx);
	kserial_printf ("rsi: 0x%llx\n", registers->rsi);
	kserial_printf ("rdi: 0x%llx\n", registers->rdi);
	kserial_printf ("rbp: 0x%llx\n\n", registers->rbp);

	kserial_printf ("r8: 0x%llx\n", registers->r8);
	kserial_printf ("r9: 0x%llx\n", registers->r9);
	kserial_printf ("r10: 0x%llx\n", registers->r10);
	kserial_printf ("r11: 0x%llx\n", registers->r11);
	kserial_printf ("r12: 0x%llx\n", registers->r12);
	kserial_printf ("r13: 0x%llx\n", registers->r13);
	kserial_printf ("r14: 0x%llx\n", registers->r14);
	kserial_printf ("r15: 0x%llx\n", registers->r15);

	kserial_printf ("----------------------------------\n\n");
}

void kernel_dispatch_interrupt (registers_t* registers) {
	irq_handler_t handler = interrupt_handlers[registers->interrupt_number];
	if (handler) {
		handler (registers);
	} else {
		kserial_printf ("Unhandled interrupt! Hasta la vista");
		log_registers_to_serial (registers);
		while (1)
			;
	}
}

void idt_register_handler (int vector, irq_handler_t handler) {
	interrupt_handlers[vector] = handler;
}

void idt_set_flags (int vector, uint8_t gate_type, uint8_t dpl, uint8_t ist) {
	idt_entry_t* descriptor = &idt[vector];

	descriptor->gate_type = gate_type;
	descriptor->dpl = dpl;
	descriptor->ist = ist;
}

void init_idt (void) {
	idtr.size = (uint16_t)(sizeof (idt_entry_t) * 256) - 1;
	idtr.offset = (uint64_t)&idt[0];

	// currently setup is same for all gates
	for (int vector = 0; vector < 256; vector++) {
		idt_set_gate (vector, isr_stub_table[vector], 0x0E, 0, 1);
		interrupt_handlers[vector] = 0;
	}

	__asm__ volatile ("lidt %0" : : "m"(idtr));
}
