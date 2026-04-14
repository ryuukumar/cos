#pragma once

#include <kernel/idt.h>

uint64_t get_current_tick (void);

void timer_handler (registers_t* registers);
void init_timer (void);
