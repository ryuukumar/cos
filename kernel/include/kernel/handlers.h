
#ifndef HANDLERS_H
#define HANDLERS_H

#include <kernel/idt.h>

void handle_gpf (registers_t* registers);

void __init_handlers__ ();

#endif