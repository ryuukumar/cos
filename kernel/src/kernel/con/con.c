/*
 * con.c
 * Copyright (C) 2026  Aditya Kumar
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, see <https://www.gnu.org/licenses/>.
 */

#include <kernel/con/ansi.h>
#include <kernel/con/con.h>
#include <kernel/con/con_ds.h>
#include <kernel/error.h>

static console_t* console = nullptr;
static bool		  update_flag = true;

static bool in_esc = false;

bool con_update_cache_set (void) {
	bool cached = update_flag;
	update_flag = true;
	return cached;
}

bool con_update_cache_clear (void) {
	bool cached = update_flag;
	update_flag = false;
	return cached;
}

void con_update_upd (bool cached) { update_flag = cached; }

int con_update (void) { return write_to_gfx (&console); }

void con_scrollup (size_t howmuch) {
	console_scrollup (&console, howmuch);
	write_to_gfx (&console);
}

void con_scrolldown (size_t howmuch) {
	console_scrolldown (&console, howmuch);
	write_to_gfx (&console);
}

int con_tiocgwinsz (winsize_t* ptr) {
	if (!ptr) return -EINVAL;

	console_parameters_t params = console_getparams (&console);
	ptr->ws_col = params.width;
	ptr->ws_row = params.height;
	ptr->ws_xpixel = (unsigned short)(params.width * (params.glyph_width + params.char_spacing) *
									  params.font_size);
	ptr->ws_ypixel = (unsigned short)(params.height * (params.glyph_height + params.line_spacing) *
									  params.font_size);

	return 0;
}

int add_char (unsigned char c) {
	int					 error = 0;
	console_parameters_t params = console_getparams (&console);
	if (in_esc) {
		ansi_status_t status = add_to_ansi_parser_buf (c);
		if (status == ANSI_INVALID) {
			const char* buf = get_ansi_buffer ();
			for (size_t i = 0; buf[i]; i++)
				console_putchar (&console, (unsigned char)buf[i]);
			clear_ansi_buffer ();
			in_esc = false;
		} else if (status == ANSI_VALID) {
			in_esc = false;
		}
	} else if (c == '\x7F') {
		idx_t idx = console_getidx (&console);
		if (CON_IDX_X (idx) == 0)
			idx = CON_IDX_GEN (params.width - 1, CON_IDX_Y (idx) - 1);
		else
			idx = CON_IDX_GEN (CON_IDX_X (idx) - 1, CON_IDX_Y (idx));

		console_goto (&console, CON_IDX_X (idx), CON_IDX_Y (idx));
		error = console_putchar (&console, 0);
		console_goto (&console, CON_IDX_X (idx), CON_IDX_Y (idx));
	} else if (c == '\r') {
		idx_t idx = console_getidx (&console);
		console_goto (&console, 0, CON_IDX_Y (idx));
	} else if (c == '\t') {
		idx_t  idx = console_getidx (&console);
		size_t x_next_tabstop = TAB_WIDTH * ((CON_IDX_X (idx) + TAB_WIDTH - 1) / TAB_WIDTH);
		if (x_next_tabstop >= params.width)
			if (CON_IDX_Y (idx) + 1 == params.height)
				console_putchar (&console, '\n');
			else
				console_goto (&console, 0, CON_IDX_Y (idx) + 1);
		else
			console_goto (&console, x_next_tabstop, CON_IDX_Y (idx));
	} else if (c == '\033') {
		in_esc = true;
	} else {
		error = console_putchar (&console, c);
	}

	if (update_flag) write_to_gfx (&console);
	return error;
}

void init_con (size_t screen_width, size_t screen_height, size_t x_padding, size_t y_padding,
			   size_t char_spacing, size_t line_padding, size_t font_multiplier) {
	console_parameters_t params = {
		.glyph_height = 8,
		.glyph_width = 5,
		.line_spacing = line_padding,
		.char_spacing = char_spacing,
		.xpad = x_padding,
		.ypad = y_padding,
		.width = (screen_width - 2 * x_padding) / ((5 + char_spacing) * font_multiplier),
		.height = (screen_height - 2 * y_padding) / ((8 + line_padding) * font_multiplier),
		.font_size = font_multiplier};

	init_ansi_buffer (&console);
	console_create (&console, &params);
};
