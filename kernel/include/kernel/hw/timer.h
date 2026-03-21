#ifndef TIMER_H
#define TIMER_H

#include <kernel/idt.h>

uint64_t get_current_tick (void);

registers_t* timer_handler (registers_t* registers);
void		 init_timer (void);

#endif