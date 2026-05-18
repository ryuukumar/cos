/*
 * Copyright (C) 2025  Aditya Kumar
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define TAB_WIDTH   8

void set_update_on_putch (bool);
bool get_update_on_putch ();
void set_idx (size_t);
size_t get_idx ();

void __init_console__ (size_t, size_t, size_t, size_t, size_t, size_t, size_t);
void set_color (uint32_t);
void update();

void putchar(unsigned char);
void putstr(const char*, size_t);

#endif