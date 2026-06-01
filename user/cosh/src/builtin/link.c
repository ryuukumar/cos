/*
 * link.c
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
#include <string.h>
#include <unistd.h>

int builtin_ln (int argc, char** argv) {
	int symbolic = 0;
	int argi = 1;

	if (argc > 1 && strcmp (argv[1], "-s") == 0) {
		symbolic = 1;
		argi = 2;
	}

	if (argc - argi != 2) {
		printf ("usage: ln [-s] target linkname\n");
		return 64;
	}

	if (symbolic)
		return symlink (argv[argi], argv[argi + 1]);
	else
		return link (argv[argi], argv[argi + 1]);
}