/*
 * readlink.c
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

int do_readlink (const char* path, char* buf, size_t bufsz) {
	if (!path || !buf) return -EINVAL;

	process* current = get_current_process ();
	inode*	 parent = nullptr;
	char *	 name = nullptr, *path_norm = nullptr;

	int error = path_normalise_from_user (path, &path_norm);
	if (error < 0) return error;
	error =
		resolve_parent_and_childname (path_norm, current->p_root, current->p_wd, &parent, &name);
	kfree (path_norm);
	if (error) goto cleanup;

	inode* node = nullptr;
	error = parent->i_iops->lookup (name, &node, parent);
	if (error) goto cleanup;

	if (node->i_type != LINK) return -EINVAL;
	if (!node->i_iops->readlink) return -ENOSYS;
	error = node->i_iops->readlink (node, buf, bufsz);

cleanup:
	kfree (name);
	return error;
}

uint64_t sys_readlink (uint64_t path, uint64_t buf, uint64_t bufsz) {
	char* p = kstrdup ((char*)path);
	int	  ret = do_readlink (p, (char*)buf, (size_t)bufsz);
	kfree (p);
	return ret;
}