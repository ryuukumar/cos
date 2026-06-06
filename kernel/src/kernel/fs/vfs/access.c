/*
 * access.c
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

int do_access (const char* path, uint8_t flags) {
	(void)flags;

	process* current = get_current_process ();
	inode*	 result = nullptr;

	int error = lookup_inode_by_path (path, current->p_root, current->p_wd, &result, L_FLNK);
	if (error) return error;

	bool access_ok = true;

	// TODO: check for permissions, when implemented

	return access_ok;
}

uint64_t sys_access (uint64_t path, uint64_t flags) {
	char* path_norm = nullptr;

	int error = path_normalise_from_user ((char*)path, &path_norm);
	if (error < 0) return error;

	error = do_access (path_norm, (uint8_t)(flags & 0xF));

	kfree ((void*)path_norm);
	return error;
}
