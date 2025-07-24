#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <limine.h>

struct posn {
	size_t x;
	size_t y;
};

extern void __init_graphics__ (struct limine_framebuffer *);
struct posn itopos(int);
size_t postoi (int, int);

void renderGlyph(unsigned char*, int, int, size_t, size_t, int, uint32_t);

void drawBorder(size_t);

#endif