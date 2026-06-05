/*
 * rmdir.c
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

int do_rmdir (const char* path) {
	if (!path) return -EINVAL;

	process* current = get_current_process ();
	inode *	 parent = nullptr, *node = nullptr;
	char*	 name = nullptr;

	int error =
		resolve_parent_and_childname ((char*)path, current->p_root, current->p_wd, &parent, &name);
	if (error == -INTERNAL_ENOPARENT) return -EINVAL;
	if (error < 0) return error;

	error = lookup_inode_by_path (name, current->p_root, parent, &node,
								  L_DCHK | (error == 1 ? L_FLNK : 0));
	if (error) goto cleanup;

	if (node->i_type != DIRECTORY) {
		error = -ENOTDIR;
		goto cleanup;
	}

	if (!node->i_iops || !node->i_iops->rmdir || !node->i_iops->empty) {
		error = -ENOSYS;
		goto cleanup;
	}

	if (!node->i_iops->empty (node)) {
		error = -ENOTEMPTY;
		goto cleanup;
	}

	if (node == current->p_wd) {
		error = -EINVAL;
		goto cleanup;
	}

	error = node->i_iops->rmdir (parent, node);
cleanup:
	kfree (name);
	return error;
}

uint64_t sys_rmdir (uint64_t path) {
	char* path_us = nullptr;
	int	  error = path_normalise_from_user ((const char*)path, &path_us);
	if (error < 0) return error;

	error = do_rmdir (path_us);
	kfree ((void*)path_us);
	return error;
}
