/*
 * chown.c
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

#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>

int do_chown (const char* path, uint64_t uid, uint64_t gid) {
	if (!path) return -EINVAL;
	inode*	 node = nullptr;
	process* current = get_current_process ();

	char* norm_path = nullptr;
	int	  error = path_normalise_from_user (path, &norm_path);
	if (error < 0) return error;
	error = lookup_inode_by_path ((char*)norm_path, current->p_root, current->p_wd, &node, L_FLNK);
	kfree (norm_path);
	if (error != 0) return error;

	node->i_uid = uid;
	node->i_gid = gid;

	return 0;
}

uint64_t sys_chown (uint64_t path, uint64_t uid, uint64_t gid) {
	return (uint64_t)do_chown ((const char*)path, uid, gid);
}
