#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TAB_WIDTH 8

typedef union {
	uint32_t raw;
	struct __attribute__((packed)) {
		uint8_t		  dirty : 1;
		unsigned char character : 7;
		uint8_t		  red;
		uint8_t		  green;
		uint8_t		  blue;
	};
} console_char_t;

void   set_update_on_putch (bool);
bool   get_update_on_putch (void);
void   set_idx (size_t);
size_t get_idx (void);

void init_console (size_t, size_t, size_t, size_t, size_t, size_t, size_t);
void set_color (uint32_t);
void update (void);

void putchar (unsigned char);
void putstr (const char*, size_t);
