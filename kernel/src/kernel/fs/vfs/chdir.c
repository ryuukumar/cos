/*
 * chdir.c
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

int do_chdir (const char* path) {
	if (!path) return -EINVAL;
	inode*	 new_dir = nullptr;
	process* current = get_current_process ();

	int error = do_lookup ((char*)path, &new_dir, current->p_root, current->p_wd);
	if (error != 0) return error;

	if (new_dir->i_type != DIRECTORY) return -ENOTDIR;
	current->p_wd = new_dir;
	return 0;
}

uint64_t sys_chdir (uint64_t path) { return (uint64_t)do_chdir ((const char*)path); }
