
#include <kernel/idt.h>

#include <stdio.h>

__attribute__((aligned(0x10)))
static idt_entry_t idt[256];

static idtr_t idtr;

irq_handler_t interrupt_handlers[256];

extern void* isr_stub_table[];

void idt_set_gate(uint8_t vector, void* isr, uint8_t gate_type, uint8_t dpl, uint8_t present) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->offset_1 = (uint64_t)isr && 0xFFFF;
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

static inline void log_reg(const char* name, uint64_t value) {
    char buf[32];
    write_serial_str(name);
    write_serial_str(": 0x");
    ulitos(value, buf, 16);
    write_serial_str(buf);
    write_serial_str("\n");
}

static void log_registers_to_serial(registers_t* registers) {
    write_serial_str("\n--- Interrupt/Exception Caught ---\n");

    log_reg("interrupt_number", registers->interrupt_number);
    log_reg("error_code", registers->error_code);
    log_reg("rip", registers->rip);
    log_reg("cs", registers->cs);
    log_reg("rflags", registers->rflags);
    log_reg("rsp", registers->rsp);
    log_reg("ss", registers->ss);
    
    write_serial_str("\n");
    
    log_reg("rax", registers->rax);
    log_reg("rbx", registers->rbx);
    log_reg("rcx", registers->rcx);
    log_reg("rdx", registers->rdx);
    log_reg("rsi", registers->rsi);
    log_reg("rdi", registers->rdi);
    log_reg("rbp", registers->rbp);
    
    write_serial_str("\n");

    log_reg("r8", registers->r8);
    log_reg("r9", registers->r9);
    log_reg("r10", registers->r10);
    log_reg("r11", registers->r11);
    log_reg("r12", registers->r12);
    log_reg("r13", registers->r13);
    log_reg("r14", registers->r14);
    log_reg("r15", registers->r15);
    
    write_serial_str("----------------------------------\n\n");
}

void kernel_dispatch_interrupt(registers_t* registers) {
    log_registers_to_serial(registers);

    irq_handler_t handler = interrupt_handlers[registers->interrupt_number];
    if (handler) handler(registers);
    else {
        printf("Unhandled interrupt! Hasta la vista");
        while(1);
    }
}

void idt_register_handler(int vector, irq_handler_t handler) {
    interrupt_handlers[vector] = handler;
}

void __init_idt__ (void) {
    idtr.size = (uint16_t)(sizeof(idt_entry_t) * 256) - 1;
    idtr.offset = (uint64_t)&idt[0];

    // currently setup is same for all gates
    for (int vector = 0; vector < 256; vector++) {
        idt_set_gate(vector, isr_stub_table[vector], 0x0E, 0, 1);
        interrupt_handlers[vector] = 0;
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr));
}
