/*
 * ansi.h
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

#include <kernel/con/con_ds.h>

typedef enum { ANSI_INCOMPLETE, ANSI_INVALID, ANSI_VALID } ansi_status_t;

ansi_status_t add_to_ansi_parser_buf (unsigned char c);

const char* get_ansi_buffer (void);
void		clear_ansi_buffer (void);
void		init_ansi_buffer (console_t** console);
