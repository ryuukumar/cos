#include <kclib/string.h>
#include <kernel/con/con_ds.h>
#include <kernel/error.h>
#include <kernel/graphics.h>
#include <kernel/hardfonts/classic.h>
#include <liballoc/liballoc.h>
#include <stddef.h>
#include <utils/deque.h>

#define CON_SCROLLBACK_LIMIT 10000

/*!
 * Increment idx_t index.
 * @param idx index to be incremented
 * @param width width of the console
 * @return incremented idx_t index
 */
static inline idx_t increment_idx (idx_t idx, size_t width) {
	return CON_IDX_GEN (CON_IDX_X (idx) == width - 1 ? 0 : CON_IDX_X (idx) + 1,
						CON_IDX_Y (idx) + (CON_IDX_X (idx) == width - 1));
}

/*!
 * Allocate and initialise a new console object.
 * @param console pointer to console_t* which will hold the reference to created console
 * @param params initialisation parameters for the console
 * @return 0 if successful, else -EINVAL or -ENOMEM
 */
int console_create (console_t** console, console_parameters_t* params) {
	if (!console || !params) return -EINVAL;

	console_t* new_console = kmalloc (sizeof (console_t));
	if (!new_console) return -ENOMEM;

	kmemset (new_console, 0, sizeof (console_t));

	new_console->scrollback = deque_create (0);
	new_console->scrollfront = deque_create (0);
	if (!new_console->scrollback || !new_console->scrollfront) return -ENOMEM;

	new_console->display = kmalloc (params->height * sizeof (console_line_t*));
	if (!new_console->display) return -ENOMEM;

	for (size_t i = 0; i < params->height; i++) {
		new_console->display[i] = kmalloc (sizeof (console_line_t));
		if (!new_console->display[i]) return -ENOMEM;

		new_console->display[i]->chars = kmalloc (params->width * sizeof (console_char_t));
		if (!new_console->display[i]->chars) return -ENOMEM;
		kmemset (new_console->display[i]->chars, 0, params->width * sizeof (console_char_t));

		new_console->display[i]->dirty = 0;
		new_console->display[i]->width = params->width;
	}

	new_console->current_color.red = 0xFF;
	new_console->current_color.green = 0xFF;
	new_console->current_color.blue = 0xFF;
	new_console->params = *params;

	*console = new_console;

	return 0;
}

/*!
 * Deallocate an existing console object.
 * @param console pointer to console_t* which holds the reference to console
 * @return 0 if successful, else -EINVAL
 */
int console_delete (console_t** console) {
	if (!console || !(*console)) return -EINVAL;

	while (deque_size ((*console)->scrollback)) {
		console_line_t* popped = nullptr;
		int				error = deque_pop_front ((*console)->scrollback, (deque_elem*)&popped);
		if (error) break;

		kfree (popped->chars);
		kfree (popped);
	}

	while (deque_size ((*console)->scrollfront)) {
		console_line_t* popped = nullptr;
		int				error = deque_pop_front ((*console)->scrollfront, (deque_elem*)&popped);
		if (error) break;

		kfree (popped->chars);
		kfree (popped);
	}

	deque_destroy ((*console)->scrollback);
	deque_destroy ((*console)->scrollfront);

	for (size_t i = 0; i < (*console)->params.height; i++) {
		kfree ((*console)->display[i]->chars);
		kfree ((*console)->display[i]);
	}

	kfree ((*console)->display);
	kfree (*console);
	*console = nullptr;

	return 0;
}

/*!
 * Write a character at the current cursor position. Increments idx to the sequentially next value.
 *
 * In the case that character \n is supplied, no update is made to actual console charaters, instead
 * idx is directly set to (0, height), potentially falling through to the following case. This is
 * the canonical way of adding new lines to the console.
 *
 * In the case that after incrementing, the condition idx.y >= height holds true, new lines are
 * added at the bottom and the console is scrolled to accomodate until the condition is no longer
 * true. In the case that after addition of a new line, the scrollback exceeds CON_SCROLLBACK_LIMIT,
 * the oldest line is freed until the condition is no longer true.
 *
 * This function does not call write_to_gfx.
 *
 * @param console pointer to console_t* which holds the reference to console
 * @param c character to put at index
 * @return 0 if successful, else -EINVAL, -ENOMEM or other errors
 */
int console_putchar (console_t** console, unsigned char c) {
	if (!console || !(*console)) return -EINVAL;

	// clear the scrollfront so we are always printing on the newest line
	console_scrolldown (console, deque_size ((*console)->scrollfront));

	if (c == '\n') {
		(*console)->idx = CON_IDX_GEN (0, (*console)->params.height);
	} else {
		console_char_t* target =
			&(*console)->display[CON_IDX_Y ((*console)->idx)]->chars[CON_IDX_X ((*console)->idx)];
		target->character = c;
		target->color = (*console)->current_color;
		(*console)->display[CON_IDX_Y ((*console)->idx)]->dirty = 1;
		(*console)->idx = increment_idx ((*console)->idx, (*console)->params.width);
	}

	while (CON_IDX_Y ((*console)->idx) >= (*console)->params.height) {
		console_line_t* new_line = kmalloc (sizeof (console_line_t));
		if (!new_line) return -ENOMEM;

		new_line->chars = kmalloc ((*console)->params.width * sizeof (console_char_t));
		if (!new_line->chars) return -ENOMEM;
		kmemset (new_line->chars, 0, (*console)->params.width * sizeof (console_char_t));

		new_line->dirty = 0;
		new_line->width = (*console)->params.width;

		int error = deque_push_front ((*console)->scrollfront, (deque_elem)new_line);
		if (error) return error;

		console_scrolldown (console, 1);
		(*console)->idx =
			CON_IDX_GEN (CON_IDX_X ((*console)->idx), CON_IDX_Y ((*console)->idx) - 1);
	}

	while (deque_size ((*console)->scrollback) > CON_SCROLLBACK_LIMIT) {
		console_line_t* buf = nullptr;
		int				error = deque_pop_back ((*console)->scrollback, (deque_elem*)&buf);
		if (error == -INTERNAL_EEMPQ)
			break;
		else if (error != 0)
			return error;

		kfree (buf->chars);
		kfree (buf);
	}

	return 0;
}

/*!
 * Set the color that will be used by putchar for following characters.
 * @param console pointer to console_t* which holds the reference to console
 * @param red 8-bit value for red channel
 * @param green 8-bit value for green channel
 * @param blue 8-but value for blue channel
 * @return 0 if successful, else -EINVAL
 */
int console_setcolor (console_t** console, uint8_t red, uint8_t green, uint8_t blue) {
	if (!console || !(*console)) return -EINVAL;
	(*console)->current_color.red = red;
	(*console)->current_color.green = green;
	(*console)->current_color.blue = blue;
	return 0;
}

/*!
 * Move the cursor to another position.
 * @param console pointer to console_t* which holds the reference to console
 * @param x x-index to seek (left-right)
 * @param y y-index to seek (top-down)
 * @return 0 if successful, else -EINVAL
 */
int console_goto (console_t** console, uint32_t x, uint32_t y) {
	if (!console || !(*console)) return -EINVAL;
	(*console)->idx = CON_IDX_GEN (x, y);
	return 0;
}

/*!
 * Scroll the whole console to reveal a line from the scrollback deque.
 *
 * If there are fewer lines in the scrollback than requested by the howmuch parameter, no new lines
 * will be created and scrolling will terminate once the scrollback deque is empty.
 *
 * @param console pointer to console_t* which holds the reference to console
 * @param howmuch how many lines to scroll
 * @return 0 if successful, else -EINVAL or other errors
 */
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
								  (deque_elem)(*console)->display[(*console)->params.height - 1]);
		if (error) return error;

		for (size_t j = (*console)->params.height - 1; j > 0; j--) {
			(*console)->display[j] = (*console)->display[j - 1];
			(*console)->display[j]->dirty = 1;
		}

		(*console)->display[0] = popped;
		(*console)->display[0]->dirty = 1;
	}

	return 0;
}

/*!
 * Scroll the whole console to reveal a line from the scrollfront deque.
 *
 * If there are fewer lines in the scrollfront than requested by the howmuch parameter, no new lines
 * will be created and scrolling will terminate once the scrollfront deque is empty.
 *
 * @param console pointer to console_t* which holds the reference to console
 * @param howmuch how many lines to scroll
 * @return 0 if successful, else -EINVAL or other errors
 */
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

		for (size_t j = 0; j < (*console)->params.height - 1; j++) {
			(*console)->display[j] = (*console)->display[j + 1];
			(*console)->display[j]->dirty = 1;
		}

		(*console)->display[(*console)->params.height - 1] = popped;
		(*console)->display[(*console)->params.height - 1]->dirty = 1;
	}

	return 0;
}

/*!
 * Free the scrollback deque.
 * @param console pointer to console_t* which holds the reference to console
 * @return 0 if successful, else -EINVAL or other errors
 */
int console_clearscrollback (console_t** console) {
	if (!console || !(*console)) return -EINVAL;

	console_line_t* popped = nullptr;
	do {
		int error = deque_pop_front ((*console)->scrollback, (deque_elem*)&popped);
		if (error == -INTERNAL_EEMPQ)
			break;
		else if (error != 0)
			return error;
		kfree (popped->chars);
		kfree (popped);
	} while (1);

	return 0;
}

/*!
 * Get the position of the cursor. Behavior undefined if console parameter is invalid
 * @param console pointer to console_t* which holds the reference to console
 * @return index
 */
idx_t console_getidx (console_t** console) { return (*console)->idx; }

/*!
 * Get the color that will be used when printing the following characters. Behavior undefined if
 * console parameter is invalid
 * @param console pointer to console_t* which holds the reference to console
 * @return color
 */
console_color_t console_getcolor (console_t** console) { return (*console)->current_color; }

/*!
 * Update the screenbuffer to match the contents of the console structure's internal display.
 * @param console pointer to console_t* which holds the reference to console
 * @return 0 if successful, else -EINVAL
 */
int write_to_gfx (console_t** console) {
	if (!console || !(*console)) return -EINVAL;

	console_parameters_t* params = &(*console)->params;
	for (size_t i = 0; i < params->height; i++) {
		if (!(*console)->display[i]->dirty) continue;
		console_char_t* target = (*console)->display[i]->chars;

		for (size_t j = 0; j < params->width; j++) {
			renderGlyph (glyph (target[j].character), 8, 5,
						 params->xpad + (params->font_size * j * (5 + params->char_spacing)),
						 params->ypad + (params->font_size * i * (8 + params->line_spacing)),
						 params->font_size, CON_COL_RGB (target[j].color));
		}

		(*console)->display[i]->dirty = 0;
	}

	return 0;
}
