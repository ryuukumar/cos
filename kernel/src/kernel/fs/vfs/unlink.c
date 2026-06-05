/*
 * unlink.c
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
#include <stddef.h>

int do_unlink (const char* path) {
	if (!path) return -EINVAL;

	process* current = get_current_process ();
	inode *	 parent = nullptr, *node = nullptr;
	char*	 name = nullptr;

	int error =
		resolve_parent_and_childname ((char*)path, current->p_root, current->p_wd, &parent, &name);
	int trailing = error == 1;
	kserial_printf ("Unlink: child name is %s and trailing slash says %i.\n", name, trailing);
	if (error < 0) return error;

	if (trailing) {
		error = lookup_inode_by_path (name, current->p_root, parent, &node, L_FLNK);
		kfree (name);
		if (error) return error;
		if (node->i_type == DIRECTORY) return -EISDIR;
		return -ENOTDIR;
	}

	error = parent->i_iops->lookup (name, &node, parent);
	if (error) {
		kfree (name);
		return error;
	}

	if (node->i_type == DIRECTORY) {
		kfree (name);
		return -EISDIR;
	}

	if (!node->i_iops || !node->i_iops->unlink) {
		kfree (name);
		return -ENOSYS;
	}

	error = node->i_iops->unlink (parent, name, node);
	kfree (name);
	return error;
}

uint64_t sys_unlink (uint64_t path) {
	char* path_us = nullptr;
	int	  error = path_normalise_from_user ((const char*)path, &path_us);
	if (error < 0) return error;

	error = do_unlink (path_us);
	kfree ((void*)path_us);
	return error;
}
