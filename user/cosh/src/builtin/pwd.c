/*
 * pwd.c
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int builtin_pwd (int argc, char** argv) {
	(void)argc, (void)argv;
	char* pathbuffer = malloc (1000);
	if (!pathbuffer) return -127;

	getcwd (pathbuffer, 1000);
	printf ("%s\n", pathbuffer);
	return 0;
}