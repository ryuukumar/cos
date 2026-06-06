/*
 * symlink.c
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

int do_symlink (const char* restrict target, const char* restrict linkpath) {
	if (!target || !linkpath) return -EINVAL;

	process* current = get_current_process ();
	inode*	 parent = nullptr;
	char*	 name = nullptr;

	int error = resolve_parent_and_childname ((char*)linkpath, current->p_root, current->p_wd,
											  &parent, &name);
	if (error) return error;

	inode* existing = nullptr;
	if (parent->i_iops->lookup (name, &existing, parent) == 0) {
		error = -EEXIST;
		goto cleanup;
	}

	if (!parent->i_iops->symlink) {
		error = -ENOSYS;
		goto cleanup;
	}

	inode* result = nullptr;
	error = parent->i_iops->symlink ((char*)target, name, &result, parent);

cleanup:
	kfree (name);
	return error;
}

uint64_t sys_symlink (uint64_t target, uint64_t linkpath) {
	char *target_us = nullptr, *linkpt_us = nullptr;

	int error = path_normalise_from_user ((char*)target, &target_us);
	if (error < 0) return error;
	error = path_normalise_from_user ((char*)linkpath, &linkpt_us);
	if (error < 0) return error;

	error = do_symlink (target_us, linkpt_us);

	kfree ((void*)target_us), kfree ((void*)linkpt_us);
	return error;
}
