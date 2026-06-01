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
	int	   error = do_lookup ((char*)oldpath, &existing, current->p_root, current->p_wd);
	if (error) return error;

	if (existing->i_type == LINK) {
		error = do_lookup ((char*)existing->i_pvt, &existing, current->p_root, current->p_wd);
		if (error) return error;
	}

	if (existing->i_type == DIRECTORY) return -EPERM;

	inode* parent = nullptr;
	char*  name = nullptr;
	error = vfs_resolve_parent (newpath, current->p_root, current->p_wd, &parent, &name);
	if (error) return error;

	// TODO: -EXDEV if on different devices/filesystems

	inode* check = nullptr;
	if (parent->i_iops->lookup (name, &check, parent) == 0) {
		kfree (name);
		return -EEXIST;
	}

	if (!parent->i_iops->link) {
		kfree (name);
		return -ENOSYS;
	}

	error = parent->i_iops->link (existing, name, parent);
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