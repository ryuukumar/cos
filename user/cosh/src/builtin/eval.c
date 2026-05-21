/*
 * eval.c
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
#include <repl.h>
#include <stdlib.h>
#include <string.h>

int builtin_eval (int argc, char** argv) {
	size_t len = 0;
	for (int i = 1; i < argc; i++)
		len += strlen (argv[i]) + 1;
	char* line = malloc (len + 1);
	line[0] = '\0';

	for (int i = 1; i < argc; i++) {
		strcat (line, argv[i]);
		if (i < argc - 1) strcat (line, " ");
	}

	int ret = run_line (line);
	free (line);
	return ret;
}
