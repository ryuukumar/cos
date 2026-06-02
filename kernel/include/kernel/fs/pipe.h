/*
 * pipe.h
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

#pragma once

#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <stdint.h>
#include <utils/charqueue.h>

struct pipe_info {
	charqueue*	  buf;
	uint32_t	  write_refs, read_refs;
	process_queue read_wait;
};

uint64_t sys_pipe (uint64_t pipefd_ptr);
