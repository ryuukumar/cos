/*
 * vfs_resolve_parent.c
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
#include <kernel/process.h>
#include <liballoc/liballoc.h>

/**
 * Resolves a path to its parent directory and returns the trailing component name.
 * @param path_arg path to resolve
 * @param root root node for the process
 * @param r_parent pointer to inode* to store the result parent
 * @param r_name pointer to char* to store the result component name
 */
int vfs_resolve_parent (const char* path_arg, inode* root, inode* cwd, inode** r_parent,
						char** r_name) {
	if (!path_arg || path_arg[0] == 0) return -EINVAL;

	char* path = kstrdup (path_arg);
	if (!path) return -ENOMEM;

	char* last_slash = kstrrchr (path, '/');

	if (!last_slash) {
		*r_name = kstrdup (path_arg);
		*r_parent = cwd;
		kfree (path);
		return cwd ? 0 : -EINVAL;
	}

	*r_name = kstrdup (last_slash + 1);

	if (last_slash == path) {
		*r_parent = root;
	} else {
		*last_slash = '\0';
		inode* start = (path[0] == '/') ? root : cwd;
		int	   err = do_lookup (path, r_parent, start, cwd);
		if (err != 0) {
			kfree (path);
			kfree (*r_name);
			return err;
		}
	}

	kfree (path);
	return 0;
}
