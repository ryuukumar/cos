/*
 * con.h
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

#pragma once

#include <kernel/fs/ioctl.h>
#include <stddef.h>
#include <stdint.h>
#include <utils/varray.h>

#define TAB_WIDTH 8

bool con_update_cache_set (void);
bool con_update_cache_clear (void);
void con_update_upd (bool cached);
int	 con_update (void);

void con_scrollup (size_t howmuch);
void con_scrolldown (size_t howmuch);

int con_tiocgwinsz (winsize_t* ptr);

int	 add_char (unsigned char c);
void init_con (size_t screen_width, size_t screen_height, size_t x_padding, size_t y_padding,
			   size_t char_spacing, size_t line_padding, size_t font_multiplier);
