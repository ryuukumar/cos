/*
 * strchr.c
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
 * Finds the first occurrence of c (after conversion to char as if by (char)c) in the
 * null-terminated byte string pointed to by s (each character interpreted as unsigned char). The
 * terminating null character is considered to be a part of the string and can be found when
 * searching for '\0'.
 * @param s string to search in
 * @param c char to search for
 * @return pointer to the found character in str, or null pointer if no such character is found.
 */
char* kstrchr (char* s, int c) {
	size_t len = kstrlen (s);
	for (size_t i = 0; i <= len; i++)
		if (s[i] == (char)c) return &s[i];
	return nullptr;
}