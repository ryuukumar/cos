/*
 * stat.c
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

int do_stat (const char* restrict path, stat* restrict buf) {
	if (!path || !buf) return -EINVAL;

	process* current = get_current_process ();
	inode*	 node = nullptr;

	int error = do_lookup ((char*)path, &node, current->p_root, current->p_wd);
	if (error != 0) return error;

	if (!node->i_iops || !node->i_iops->stat) return -ENOSYS;
	return node->i_iops->stat (node, buf);
}

uint64_t sys_stat (uint64_t path, uint64_t buf) {
	const char* path_us = kstrdup ((const char*)path);
	int			error = do_stat (path_us, (stat*)buf);
	kfree ((void*)path_us);
	return error;
}
