/*
 * link.c
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

int do_link (const char* oldpath, const char* newpath) {
	if (!oldpath || !newpath) return -EINVAL;

	process* current = get_current_process ();

	inode* existing = nullptr;
	char * norm_oldpath = nullptr, *norm_newpath;
	int	   error = path_normalise_from_user (oldpath, &norm_oldpath);
	if (error < 0) return error;
	error = path_normalise_from_user (newpath, &norm_newpath);
	if (error < 0) {
		kfree (norm_oldpath);
		return error;
	}

	error = lookup_inode_by_path ((char*)norm_oldpath, current->p_root, current->p_wd, &existing,
								  L_FLNK | L_NDCHK);
	kfree (norm_oldpath);
	if (error == -EISDIR) return -EPERM;
	if (error != 0) return error;

	inode* parent = nullptr;
	char*  name = nullptr;
	error =
		resolve_parent_and_childname (norm_newpath, current->p_root, current->p_wd, &parent, &name);
	kfree (norm_newpath);
	if (error == -INTERNAL_ENOPARENT) return -EINVAL;
	if (error < 0) return error;
	if (error == 1) return -ENOENT;

	// TODO: -EXDEV if on different devices/filesystems

	inode* check = nullptr;
	error = parent->i_iops->lookup (name, &check, parent);

	if (error == 0) {
		error = -EEXIST;
		goto cleanup;
	} else if (error != -ENOENT)
		goto cleanup;

	if (!parent->i_iops->link) {
		error = -ENOSYS;
		goto cleanup;
	}

	error = parent->i_iops->link (existing, name, parent);

cleanup:
	kfree (name);
	return error;
}

uint64_t sys_link (uint64_t oldpath, uint64_t newpath) {
	char* oldpath_us = kstrdup ((char*)oldpath);
	char* newpath_us = kstrdup ((char*)newpath);
	int	  error = do_link (oldpath_us, newpath_us);
	kfree (oldpath_us), kfree (newpath_us);
	return error;
}