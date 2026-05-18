/*
 * memset.c
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
 * Copies the value (unsigned char)ch into each of the first count characters of the object pointed
 * to by dest.
 *
 * @param s pointer to the buffer to fill
 * @param c fill byte
 * @param n number of bytes to fill
 * @return s
 */
void* kmemset (void* s, int c, size_t n) {
	unsigned char* p = (unsigned char*)s;
	for (size_t i = 0; i < n; i++)
		p[i] = (unsigned char)c;
	return s;
}

/*!
 * Copies the value (unsigned char)ch into each of the first count characters of the object pointed
 * to by dest. Safe for sensitive information.
 *
 * @param s pointer to the buffer to fill
 * @param c fill byte
 * @param n number of bytes to fill
 * @return s
 */
void* kmemset_explicit (void* s, int c, size_t n) {
	volatile unsigned char* p = (volatile unsigned char*)s;
	for (size_t i = 0; i < n; i++)
		p[i] = (unsigned char)c;
	return s;
}