/*
 * chroot.c
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

int do_chroot (const char* path) {
	if (!path) return -EINVAL;
	inode*	 new_dir = nullptr;
	process* current = get_current_process ();

	char* norm_path = nullptr;
	int	  error = path_normalise_from_user (path, &norm_path);
	if (error < 0) return error;
	error = lookup_inode_by_path ((char*)norm_path, current->p_root, current->p_wd, &new_dir,
								  L_FLNK | L_DCHK);
	kfree (norm_path);
	if (error != 0) return error;

	current->p_root = new_dir;
	return 0;
}

uint64_t sys_chroot (uint64_t path) { return (uint64_t)do_chroot ((const char*)path); }
