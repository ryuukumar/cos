#pragma once

#include <stddef.h>
#include <stdint.h>
#include <utils/deque.h>

#define CON_IDX_X(idx)	   (idx & 0xFFFFFFFF)
#define CON_IDX_Y(idx)	   ((idx & (0xFFFFFFFFull << 32)) >> 32)
#define CON_COL_RGB(color) ((color.red << 16) & (color.green << 8) & color.blue)

typedef uint64_t idx_t;

typedef struct __attribute__ ((packed)) {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} console_color_t;

typedef struct __attribute__ ((packed)) {
	unsigned char	character;
	console_color_t color;
} console_char_t;

typedef struct __attribute__ ((packed)) {
	console_char_t* chars;
	size_t			width;
	uint8_t			dirty;
} console_line_t;

typedef struct {
	size_t glyph_height, glyph_width;
	size_t line_spacing, char_spacing;
	size_t xpad, ypad;
	size_t width, height;
	size_t font_size;
} console_parameters_t;

typedef struct __attribute__ ((packed)) {
	console_line_t**	 display;
	deque*				 scrollback;
	deque*				 scrollfront;
	idx_t				 idx;
	console_color_t		 current_color;
	console_parameters_t params;
} console_t;

int console_create (console_t** console, console_parameters_t* params);
int console_delete (console_t** console);

int console_putchar (console_t** console, unsigned char c);
int console_setcolor (console_t** console, uint8_t red, uint8_t green, uint8_t blue);
int console_goto (console_t** console, uint32_t x, uint32_t y);

int console_scrollup (console_t** console, size_t howmuch);
int console_scrolldown (console_t** console, size_t howmuch);
int console_clearscrollback (console_t** console);

idx_t			console_getidx (console_t** console);
console_color_t console_getcolor (console_t** console);

int write_to_gfx (console_t** console);
