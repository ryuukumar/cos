#include "utils/deque.h"
#include <kclib/string.h>
#include <kernel/con/con_ds.h>
#include <kernel/error.h>
#include <liballoc/liballoc.h>
#include <stddef.h>

#define CON_SCROLLBACK_LIMIT 10000

int console_create (console_t** console, size_t width, size_t height) {
	if (!console) return -EINVAL;

	console_t* new_console = kmalloc (sizeof (console_t));
	if (!new_console) return -ENOMEM;

	kmemset (new_console, 0, sizeof (console_t));

	new_console->scrollback = deque_create (0);
	new_console->scrollfront = deque_create (0);
	if (!new_console->scrollback || !new_console->scrollfront) return -ENOMEM;

	new_console->display = kmalloc (height * sizeof (console_line_t*));
	if (!new_console->display) return -ENOMEM;

	for (size_t i = 0; i < height; i++) {
		new_console->display[i] = kmalloc (sizeof (console_line_t));
		if (!new_console->display[i]) return -ENOMEM;

		new_console->display[i]->chars = kmalloc (width * sizeof (console_char_t));
		if (!new_console->display[i]->chars) return -ENOMEM;

		new_console->display[i]->dirty = 0;
		new_console->display[i]->width = width;
	}

	new_console->current_color.red = 0xFF;
	new_console->current_color.green = 0xFF;
	new_console->current_color.blue = 0xFF;

	new_console->height = height;
	new_console->width = width;

	*console = new_console;

	return 0;
}

int console_delete (console_t** console) {
	if (!console || !(*console)) return -EINVAL;
	return -ENOSYS;
}

int console_putchar (console_t** console, unsigned char c) {
	if (!console || !(*console)) return -EINVAL;
	return -ENOSYS;
}

int console_setcolor (console_t** console, uint8_t red, uint8_t green, uint8_t blue) {
	if (!console || !(*console)) return -EINVAL;
	(*console)->current_color.red = red;
	(*console)->current_color.green = green;
	(*console)->current_color.blue = blue;
	return 0;
}

int console_goto (console_t** console, uint32_t x, uint32_t y) {
	if (!console || !(*console)) return -EINVAL;
	(*console)->idx = ((uint64_t)y << 32) | (uint64_t)x;
	return 0;
}

int console_scrollup (console_t** console, size_t howmuch) {
	if (!console || !(*console)) return -EINVAL;

	for (size_t i = 0; i < howmuch; i++) {
		console_line_t* popped = nullptr;
		int				error = deque_pop_front ((*console)->scrollback, (deque_elem*)&popped);
		if (error == -INTERNAL_EEMPQ)
			break;
		else if (error != 0)
			return error;

		error = deque_push_front ((*console)->scrollfront,
								  (deque_elem)(*console)->display[(*console)->height - 1]);
		if (error) return error;

		for (size_t j = (*console)->height - 1; j > 0; j--)
			(*console)->display[j] = (*console)->display[j - 1];
		(*console)->display[0] = popped;
	}

	return 0;
}

int console_scrolldown (console_t** console, size_t howmuch) {
	if (!console || !(*console)) return -EINVAL;

	for (size_t i = 0; i < howmuch; i++) {
		console_line_t* popped = nullptr;
		int				error = deque_pop_front ((*console)->scrollfront, (deque_elem*)&popped);
		if (error == -INTERNAL_EEMPQ)
			break;
		else if (error != 0)
			return error;

		error = deque_push_front ((*console)->scrollback, (deque_elem)(*console)->display[0]);
		if (error) return error;

		for (size_t j = 0; j < (*console)->height - 1; j++)
			(*console)->display[j] = (*console)->display[j + 1];
		(*console)->display[(*console)->height - 1] = popped;
	}

	return 0;
}

int console_clearscrollback (console_t** console) {
	if (!console || !(*console)) return -EINVAL;
	return -ENOSYS;
}

idx_t console_getidx (console_t** console) { return (*console)->idx; }

console_color_t console_getcolor (console_t** console) { return (*console)->current_color; }
