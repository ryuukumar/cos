#include <kclib/stdio.h>
#include <kernel/graphics.h>
#include <kernel/memmgt.h>
#include <stddef.h>
#include <stdint.h>

static struct limine_framebuffer* framebuffer;
static size_t					  frmw, frmh;
static size_t					  stride_pixels;
uint32_t*						  fb_ptr;

/*!
Initialise the graphics interface.

@param	buf	pointer to framebuffer passed by lumine
*/
void init_graphics (struct limine_framebuffer* buf) {
	framebuffer = buf;
	frmw = framebuffer->width;
	frmh = framebuffer->height;
	stride_pixels = framebuffer->pitch / (framebuffer->bpp / 8);
	fb_ptr = framebuffer->address;
}

/*!
Convert index to screen position.

@param	idx	index
*/
struct posn itopos (int idx) {
	struct posn ret;
	ret.x = idx % framebuffer->width;
	ret.y = idx / framebuffer->width;
	return ret;
}

/*!
Convert screen position to index.

@param	x x-position
@param	y y-position
*/
inline size_t postoi (int x, int y) { return (size_t)x + ((size_t)y * stride_pixels); }

/*!
Place a pixel on the screen.

@param	color RGB color
@param	x x-position
@param	y y-position
*/
static void putPixel (uint32_t color, int x, int y) {
	if (x < 0 || y < 0) return;
	if ((size_t)x >= frmw || (size_t)y >= frmh) return;
	fb_ptr[postoi (x, y)] = color;
}

/*!
Place a character on the screen

@param	glyph pointer to gwxgh matrix describing the glyph
@param	gh glyph height (8 for classic font)
@param	gw glyph width (5 for classic font)
@param	posx x-position where character will be printed
@param	posy y-position where character will be printed
@param	size_mult size of character as a multipler (1 for default size)
@param	color color of character
*/
void renderGlyph (unsigned char* glyph, int gh, int gw, size_t posx, size_t posy, int size_mult,
				  uint32_t color) {
	for (int i = 0; i < gh; i++)
		for (int j = 0; j < gw; j++)
			for (int kx = 0; kx < size_mult; kx++)
				for (int ky = 0; ky < size_mult; ky++)
					putPixel (glyph[i * gw + j] ? color : 0, posx + (j * size_mult) + kx,
							  posy + (i * size_mult) + ky);
}

/*!
Draw a white border around the screen.

@param	padding pixels to leave around the edges
*/
void drawBorder (size_t padding) {
	for (size_t y = 0; y < frmh; y++)
		for (size_t x = 0; x < frmw; x++)
			if (x == padding || x + padding + 1 == frmw || y == padding || y + padding + 1 == frmh)
				putPixel (0xffffff, (int)x, (int)y);
}