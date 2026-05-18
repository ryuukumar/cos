/*
 * memchr.c
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

#include <kclib/string.h>

/*!
 * Finds the first occurrence of (unsigned char)c in the initial n bytes (each interpreted as
 * unsigned char) of the object pointed to by s.
 *
 * @param s buffer to search in
 * @param c unsigned char to search for
 * @param n length of buffer to search in
 * @return pointer to the location of the byte, or a null pointer if no such byte is found.
 */
void* kmemchr (void* s, int c, size_t n) {
	unsigned char* p = (unsigned char*)s;
	for (size_t i = 0; i < n; i++)
		if (p[i] == (unsigned char)c) return (void*)(p + i);
	return nullptr;
}