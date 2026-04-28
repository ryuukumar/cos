#pragma once

#include <stdint.h>

typedef struct __attribute__ ((packed)) {
	uint16_t size;
	uint64_t offset;
} idtr_t;

typedef struct __attribute__ ((packed)) {
	uint16_t offset_1;
	uint16_t selector;
	uint8_t	 ist : 3;
	uint8_t	 zero_1 : 5;
	uint8_t	 gate_type : 4;
	uint8_t	 zero_2 : 1;
	uint8_t	 dpl : 2;
	uint8_t	 present : 1;
	uint16_t offset_2;
	uint32_t offset_3;
	uint32_t zero_3;
} idt_entry_t;

typedef struct __attribute__ ((packed)) {
	uint64_t rax, rbx, rcx, rdx, rbp, rsi, rdi;
	uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
	uint64_t interrupt_number, error_code; // pushed by us
	uint64_t rip, cs, rflags, rsp, ss;
} registers_t;

typedef void (*irq_handler_t) (registers_t*);

void init_idt (void);
void idt_register_handler (int vector, irq_handler_t handler);
void idt_set_flags (int vector, uint8_t gate_type, uint8_t dpl, uint8_t ist);

void log_registers_to_serial (registers_t* registers);
