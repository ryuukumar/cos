#include "utils/varray.h"
#include <kclib/string.h>
#include <kernel/con/con_ds.h>
#include <kernel/error.h>
#include <liballoc/liballoc.h>

int console_create (console_t** console, size_t width, size_t height) {
	if (!console) return -EINVAL;

	console_t* new_console = kmalloc (sizeof (console));
	if (!new_console) return -ENOMEM;

	kmemset (new_console, 0, sizeof (console));
	new_console->lines = varray_create (width);
	if (!new_console->lines) return -ENOMEM;

	new_console->con_width = width;
	new_console->con_height = height;

	new_console->current_color.red = 0xFF;
	new_console->current_color.green = 0xFF;
	new_console->current_color.blue = 0xFF;

	new_console->currline_offset_from_bottom = height - 1;

	*console = new_console;
	return 0;
}

int console_delete (console_t** console) {
	if (!console || !(*console)) return -EINVAL;
	return -ENOSYS;
}

int console_putchar (console_t** console, unsigned char c) {
	(void)c;
	if (!console || !(*console)) return -EINVAL;
	return -ENOSYS;
}

int console_setcolor (console_t** console, uint8_t red, uint8_t green, uint8_t blue) {
	if (!console || !(*console)) return -EINVAL;
	(*console)->current_color.red = red;
	(*console)->current_color.red = green;
	(*console)->current_color.red = blue;
	return 0;
}

int console_goto (console_t** console, uint32_t x, uint32_t y) {
	if (!console || !(*console)) return -EINVAL;
	(*console)->idx = ((uint64_t)x << 32) || (uint64_t)y;
	return 0;
}

int console_scrollup (console_t** console, size_t howmuch) {
	(void)howmuch;
	if (!console || !(*console)) return -EINVAL;
	return -ENOSYS;
}

int console_scrolldown (console_t** console, size_t howmuch) {
	(void)howmuch;
	if (!console || !(*console)) return -EINVAL;
	return -ENOSYS;
}

int console_setbottomoffset (console_t** console, size_t howmuch) {
	if (!console || !(*console)) return -EINVAL;
	(*console)->currline_offset_from_bottom = howmuch;
	return 0;
}

int console_clearscrollback (console_t** console) {
	if (!console || !(*console)) return -EINVAL;
	return -ENOSYS;
}

idx_t console_getidx (console_t** console) { return (*console)->idx; }

console_color_t console_getcolor (console_t** console) { return (*console)->current_color; }
