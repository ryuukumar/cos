/*
 * pipe.c
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
#include <kernel/fs/pipe.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>
#include <utils/charqueue.h>

static int pipe_read (inode* i, file* f, void* buf, size_t cnt) {
	(void)f;

	pipe_info_t* info = i->i_info.pipe_info;
	char*		 cbuf = (char*)buf;
	size_t		 got = 0;

	while (got < cnt) {
		unsigned char next;
		while (pop_charqueue (info->buf, &next) == 0) {
			cbuf[got++] = next;
			if (got >= cnt) return (int)got;
		}

		if (got > 0) return (int)got;
		if (info->write_refs == 0) return 0;

		process_block (&info->read_wait);
		process* cur = get_current_process ();
		if (cur->p_pending & ~cur->p_sigmask) return -EINTR;
	}
	return (int)got;
}

static int pipe_close_reader (inode* i, file* f) {
	(void)f;
	pipe_info_t* info = i->i_info.pipe_info;
	info->read_refs--;
	if (i->i_cnt == 0) {
		free_charqueue (info->buf);
		kfree (info), kfree (i);
	}
	return 0;
}

static file_operations reader_operations = {.read = pipe_read, .close = pipe_close_reader};
static file_operations writer_operations;

uint64_t sys_pipe (uint64_t pipefd_ptr) {
	process* current = get_current_process ();
	int64_t	 writer_fd = -1, reader_fd = -1;
	int*	 pipefd = (int*)pipefd_ptr;
	int		 error = -ENOMEM;

	pipe_info_t* info = kmalloc (sizeof (pipe_info_t));
	if (!info) goto enomem_none;
	kmemset (info, 0, sizeof (pipe_info_t));
	info->buf = create_charqueue ();
	if (!info->buf) goto enomem_info;
	info->write_refs = info->read_refs = 1;

	inode* pipe_inode = kmalloc (sizeof (inode));
	if (!pipe_inode) goto enomem_buf;
	kmemset (pipe_inode, 0, sizeof (inode));
	pipe_inode->i_type = PIPE;
	pipe_inode->i_info.pipe_info = info;
	pipe_inode->i_cnt = 2;

	struct file* reader_file = kmalloc (sizeof (struct file));
	if (!reader_file) goto enomem_reader;
	struct file* writer_file = kmalloc (sizeof (struct file));
	if (!writer_file) goto enomem_all;

	kmemset (reader_file, 0, sizeof (struct file));
	kmemset (writer_file, 0, sizeof (struct file));

	writer_file->f_inode = reader_file->f_inode = pipe_inode;
	writer_file->f_cnt = reader_file->f_cnt = 1;
	reader_file->f_fops = &reader_operations;
	writer_file->f_fops = &writer_operations;

	for (int64_t i = 0; i < MAX_FDS; i++) {
		if (!current->p_fds[i]) {
			if (reader_fd == -1)
				reader_fd = i;
			else
				writer_fd = i;
		}
		if (writer_fd != -1) break;
	}

	if (writer_fd < 0) goto emfile_all;

	current->p_fds[reader_fd] = reader_file;
	current->p_fds[writer_fd] = writer_file;
	pipefd[0] = reader_fd;
	pipefd[1] = writer_fd;
	return 0;

emfile_all:
	error = -EMFILE;
enomem_all:
	kfree (reader_file);
enomem_reader:
	kfree (pipe_inode);
enomem_buf:
	free_charqueue (info->buf);
enomem_info:
	kfree (info);
enomem_none:
	return error;
}
