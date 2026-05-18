/*
 * ls.c
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
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int builtin_ls (int argc, char** argv) {
	const char* path = (argc >= 2) ? argv[1] : ".";

	DIR* dir = opendir (path);
	if (!dir) {
		perror ("ls");
		return -1;
	}

	size_t count = 0, cap = 64;
	char** names = malloc (cap * sizeof (char*));
	if (!names) {
		closedir (dir);
		return -1;
	}

	size_t		   max_len = 0;
	struct dirent* entry;
	while ((entry = readdir (dir)) != NULL) {
		if (count == cap) {
			cap *= 2;
			char** tmp = realloc (names, cap * sizeof (char*));
			if (!tmp) break;
			names = tmp;
		}
		names[count] = strdup (entry->d_name);
		if (!names[count]) break;
		size_t len = strlen (entry->d_name);
		if (len > max_len) max_len = len;
		count++;
	}
	closedir (dir);

	unsigned short term_cols = 80;
	struct winsize ws;
	if (ioctl (STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) term_cols = ws.ws_col;

	size_t col_width = max_len + 2;
	size_t ncols = term_cols / col_width;
	if (ncols == 0) ncols = 1;
	size_t nrows = (count + ncols - 1) / ncols;

	for (size_t row = 0; row < nrows; row++) {
		for (size_t col = 0; col < ncols; col++) {
			size_t idx = col * nrows + row;
			if (idx >= count) break;
			int last = (col + 1 == ncols) || ((col + 1) * nrows + row >= count);
			if (last)
				printf ("%s\n", names[idx]);
			else
				printf ("%-*s", (int)col_width, names[idx]);
		}
	}

	for (size_t i = 0; i < count; i++)
		free (names[i]);
	free (names);
	return 0;
}