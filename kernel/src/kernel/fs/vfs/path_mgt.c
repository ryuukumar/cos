/*
 * path_mgt.c
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
#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <liballoc/liballoc.h>

/*!
 * Parses a path passed from userland into a more regularised path which avoids multiple slashes,
 * and exists in kernel memory. Does NOT attempt to parse '.' or '..' entries. Empty source path and
 * null pointers are rejected with -EINVAL.
 *
 * @param path The path passed from userland (or kernelland)
 * @param outpuath Pointer to a char* which will point to the parsed path. This has to be kfree'd by
 * the caller
 * @return Size of parsed path (>=0) if successful, else EINVAL, ENAMETOOLONG or ENOMEM
 */
int path_normalise_from_user (const char* path, char** outpath) {
	if (!path || path[0] == 0 || !outpath) return -EINVAL;

	// TODO: needs a safe copy from userland function

	size_t path_len = kstrnlen (path, MAX_PATHLEN);
	if (path_len == MAX_PATHLEN) return -ENAMETOOLONG;

	char* new_path = kmalloc (path_len + 1);
	if (!new_path) return -ENOMEM;
	kmemset (new_path, 0, path_len + 1);

	bool   slash_state = false;
	size_t new_path_sz = 0;

	for (size_t i = 0; i < path_len; i++) {
		if (path[i] == '/') {
			if (!slash_state) {
				slash_state = true;
				new_path[new_path_sz++] = '/';
			}
		} else {
			slash_state = false;
			new_path[new_path_sz++] = path[i];
		}
	}

	new_path = krealloc (new_path, new_path_sz + 1);
	*outpath = new_path;

	return new_path_sz + 1;
}
