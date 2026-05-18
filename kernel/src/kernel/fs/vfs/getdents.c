/*
 * getdents.c
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

int do_getdents (struct file* f, void* buf, size_t count) {
	if (!f || !buf) return -EINVAL;
	if (!f->f_fops || !f->f_fops->getdents) return -ENOSYS;
	return f->f_fops->getdents (f->f_inode, f, buf, count);
}

uint64_t sys_getdents (uint64_t fd, uint64_t buf, uint64_t count) {
	process* current = get_current_process ();
	if (fd >= MAX_FDS || !current || !current->p_fds[fd]) return -EINVAL;
	return do_getdents (current->p_fds[fd], (void*)buf, (size_t)count);
}
