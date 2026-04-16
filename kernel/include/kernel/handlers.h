#pragma once

#include <kernel/idt.h>

void handle_gpf (registers_t* registers);

void init_handlers (void);
bool is_init_gpf (void);