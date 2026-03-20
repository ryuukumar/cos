#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TAB_WIDTH 8

void   set_update_on_putch (bool);
bool   get_update_on_putch (void);
void   set_idx (size_t);
size_t get_idx (void);

void init_console (size_t, size_t, size_t, size_t, size_t, size_t, size_t);
void set_color (uint32_t);
void update (void);

void putchar (unsigned char);
void putstr (const char*, size_t);

#endif