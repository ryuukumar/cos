/*
 * touch.c
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

#include <builtin.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int builtin_touch (int argc, char** argv) {
	if (argc == 1) {
		printf ("usage: touch file");
		return -127;
	}

	int fd = open (argv[1], O_CREAT | O_WRONLY, 0644);
	if (fd < 0) {
		perror (argv[1]);
		return 1;
	}
	close (fd);
	return 0;
}
