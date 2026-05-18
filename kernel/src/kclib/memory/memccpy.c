/*
 * memccpy.c
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
 * Copies bytes from the object pointed to by s2 to the object pointed to by s1, stopping after
 * any of the next two conditions are satisfied:
 *
 * - n bytes are copied
 *
 * - the byte (unsigned char)c is found (and copied).
 *
 * The s2 and s1 objects are interpreted as arrays of unsigned char.
 *
 * @param s1 destination buffer
 * @param s2 source buffer
 * @param c character to match
 * @param n limit on number of bytes to copy
 * @return If the byte (unsigned char)c was found, memccpy returns a pointer to the next byte in
 * s1 after (unsigned char)c. Otherwise it returns a null pointer.
 */
void* kmemccpy (void* restrict s1, const void* restrict s2, int c, size_t n) {
	unsigned char*		 dest = (unsigned char*)s1;
	const unsigned char* src = (const unsigned char*)s2;
	unsigned char		 uc = (unsigned char)c;

	for (size_t i = 0; i < n; i++) {
		dest[i] = src[i];
		if (src[i] == uc) return &dest[i + 1];
	}
	return nullptr;
}