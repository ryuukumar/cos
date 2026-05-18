/*
 * fstat.c
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
#include <stddef.h>

int do_fstat (struct file* fd, stat* buf) {
	if (!fd || !buf) return -EINVAL;
	if (!fd->f_fops || !fd->f_fops->fstat) return -ENOSYS;
	return fd->f_fops->fstat (fd->f_inode, fd, buf);
}

uint64_t sys_fstat (uint64_t fd, uint64_t buf) {
	process* current = get_current_process ();
	if (fd >= MAX_FDS || !current || !current->p_fds[fd]) return -EINVAL;
	return do_fstat (current->p_fds[fd], (stat*)buf);
}
