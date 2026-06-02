/*
 * dup.c
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

uint64_t sys_dup (uint64_t fd) {
	process* p = get_current_process ();
	if (!p || fd >= MAX_FDS || !p->p_fds[fd]) return -EBADF;

	int newfd = -1;
	for (int i = 0; i < MAX_FDS; i++) {
		if (!p->p_fds[i]) {
			newfd = i;
			break;
		}
	}
	if (newfd < 0) return -EMFILE;

	p->p_fds[newfd] = p->p_fds[fd];
	p->p_fds[newfd]->f_cnt++;
	return newfd;
}

uint64_t sys_dup2 (uint64_t oldfd, uint64_t newfd) {
	process* p = get_current_process ();
	if (!p || oldfd >= MAX_FDS || newfd >= MAX_FDS || !p->p_fds[oldfd]) return -EBADF;

	if (p->p_fds[newfd]) {
		struct file* old = p->p_fds[newfd];
		p->p_fds[newfd] = nullptr;
		do_close (old);
	}

	p->p_fds[newfd] = p->p_fds[oldfd];
	p->p_fds[newfd]->f_cnt++;
	return newfd;
}
