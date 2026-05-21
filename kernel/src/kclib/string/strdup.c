/*
 * strdup.c
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
#include <liballoc/liballoc.h>

/*!
 * Returns a pointer to a null-terminated byte string, which is a duplicate of the string pointed to
 * by src. The space for the new string is obtained as if the malloc was invoked. The returned
 * pointer must be passed to kfree to avoid a memory leak.
 * @param s string to copy
 * @return pointer to new string if memory could be allocated, else nullptr
 */
char* kstrdup (const char* s) {
	size_t len = kstrlen (s) + 1; // additional byte for zero
	char*  dup = (char*)kmalloc (len);
	if (!dup) return nullptr;

	kstrcpy (dup, s);
	return dup;
}

/*!
 * Returns a pointer to a null-terminated byte string, which contains copies of at most size bytes
 * from the string pointed to by src. The space for the new string is obtained as if malloc was
 * called. If the null terminator is not encountered in the first size bytes, it is appended to the
 * duplicated string. The returned pointer must be passed to kfree to avoid a memory leak.
 * @param s string to copy
 * @param n limit on how long the string may be
 * @return pointer to new string if memory could be allocated, else nullptr
 */
char* kstrndup (const char* s, size_t n) {
	size_t len = kstrnlen (s, n);
	char*  dup = (char*)kmalloc (len + 1);
	if (!dup) return nullptr;

	kstrncpy (dup, s, len);
	dup[len] = 0;
	return dup;
}
