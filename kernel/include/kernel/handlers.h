
#ifndef HANDLERS_H
#define HANDLERS_H

#include <kernel/idt.h>

registers_t* handle_gpf (registers_t* registers);

void init_handlers (void);

#endif