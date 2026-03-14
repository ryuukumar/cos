
#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_COM_1    0x3F8

#include <stdint.h>

#include <kernel/io.h>

int __init_serial__ (void);

uint8_t read_serial (void);
void write_serial (uint8_t val);
void write_serial_str (const char* str);

#endif