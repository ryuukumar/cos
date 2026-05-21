/*
 * memmove.c
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
 * Copies n characters from the object pointed to by s2 to the object pointed to by s1. Both
 * objects are interpreted as arrays of unsigned char. The objects may overlap: copying takes place
 * as if the characters were copied to a temporary character array and then the characters were
 * copied from the array to s1.
 *
 * @param s1 destination buffer
 * @param s2 source buffer
 * @param n bytes to move
 * @return s1
 */
void* kmemmove (void* s1, const void* s2, size_t n) {
	unsigned char*		 pdest = (unsigned char*)s1;
	const unsigned char* psrc = (const unsigned char*)s2;

	if (s2 > s1)
		for (size_t i = 0; i < n; i++)
			pdest[i] = psrc[i];
	else if (s2 < s1)
		for (size_t i = n; i > 0; i--)
			pdest[i - 1] = psrc[i - 1];

	return s1;
}