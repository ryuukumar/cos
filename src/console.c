#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <hardfonts/classic.h>
#include <memory.h>
#include <console.h>
#include <string.h>

#define 	CONSOLE_HEIGHT		80
#define		CONSOLE_WIDTH		140

bool		PUTCH_UPDATE = 		false;

unsigned char bufferstr [CONSOLE_HEIGHT*CONSOLE_WIDTH] = {0};		//TODO: replace this with malloc when implemented
uint32_t colorbufferstr [CONSOLE_HEIGHT*CONSOLE_WIDTH] = {0xffffff};		//TODO: replace this with malloc when implemented
unsigned char* buffer = bufferstr;
uint32_t* colorbuffer = colorbufferstr;
size_t x, y, xs, ys, xsp, ysp, xc, yc, fs;
size_t idx = 0;

uint32_t color = 0xeeeeee;

/*!
Set PUTCH_UPDATE

@param	set value to set to
*/
void set_update_on_putch (bool set) {
	PUTCH_UPDATE = set;
}

/*!
Get PUTCH_UPDATE

@return	PUTCH_UPDATE
*/
bool get_update_on_putch () {
	return PUTCH_UPDATE;
}

/*!
Set idx

@param	set value to set to
*/
void set_idx (size_t set) {
	idx = set;
}

/*!
Get idx

@return	idx
*/
size_t get_idx () {
	return idx;
}

/*!
Initialise the console with the default (classic) font

@param	x_screen screen width
@param	y_screen screen height
@param	x_pad padding around sides
@param	y_pad padding around top and bottom
@param	x_spc spacing between characters
@param	y_spc spacing between lines
@param	font_size font size
*/
void __init_console__ (size_t x_screen, size_t y_screen, size_t x_pad, size_t y_pad, size_t x_spc, size_t y_spc, size_t font_size) {
	x = x_screen - 2 * x_pad;
	y = y_screen - 2 * y_pad;

	xc = (x/((5+x_spc)*font_size)>CONSOLE_WIDTH) ? CONSOLE_WIDTH : x/((5+x_spc)*font_size);
	yc = (y/((8+y_spc)*font_size)>CONSOLE_HEIGHT) ? CONSOLE_HEIGHT : y/((8+y_spc)*font_size);

	xsp = x_spc;
	ysp = y_spc;

	xs = x_pad;
	ys = y_pad;

	fs = font_size;

	for (int i=0; i<xc*yc; i++) buffer[i] = 32;
}

/*!
Set the screen color.

@param	c RGB color
*/
void set_color(uint32_t c) {
	color = c;
}

/*!
Update the display.
*/
void update() {
	for (int i=0; i<yc; i++) {
		for (int j=0; j<xc; j++) {
			renderGlyph(glyph(buffer[i*xc+j]), 
					8, 5, xs+(fs*j*(5+xsp)), 
					ys+(fs*i*(8+ysp)), fs,
					colorbuffer[i*xc+j]);
		}
	}
}

/*!
Register a character to the screen buffer.

@param	rc character to register
@param	index position to print it on
*/
void registerChar(unsigned char rc, int index) {
	buffer[index] = rc;
	colorbuffer[index] = color;
}

/*!
Print a single character to the screen.

@param	rc character to print
*/
void putchar(unsigned char rc) {
	switch (rc) {
		case '\n':
			idx = xc*((idx/xc)+1);
			while (idx >= CONSOLE_HEIGHT*CONSOLE_WIDTH) idx -= CONSOLE_HEIGHT*CONSOLE_WIDTH;
			return;
	}
	registerChar(rc, idx);
	idx++;
	if (PUTCH_UPDATE) update();
}

/*!
Print a string to the screen.

@param	str string to print
@param	len length of string
*/
void putstr(const char* str, size_t len) {
	for (size_t i=0; i<len; i++) putchar(str[i]);
	update();
}

