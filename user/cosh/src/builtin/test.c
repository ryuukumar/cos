/*
 * test.c
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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int test_file (const char* op, const char* path) {
	struct stat st;
	if (stat (path, &st) != 0) return 1;
	switch (op[1]) {
	case 'e':
		return 0;
	case 'f':
		return S_ISREG (st.st_mode) ? 0 : 1;
	case 'd':
		return S_ISDIR (st.st_mode) ? 0 : 1;
	case 's':
		return (st.st_size > 0) ? 0 : 1;
	default:
		return 1;
	}
}

int builtin_test (int argc, char** argv) {
	argv++, argc--;
	if (argc > 0 && strcmp (argv[argc - 1], "]") == 0) argc--;

	switch (argc) {
	case 0:
		return 1;

	case 1:
		return (argv[0][0] != '\0') ? 0 : 1;

	case 2: {
		const char* op = argv[0];
		const char* val = argv[1];
		if (strcmp (op, "!") == 0) return (val[0] != '\0') ? 1 : 0;
		if (op[0] == '-' && op[2] == '\0') switch (op[1]) {
			case 'z':
				return (strlen (val) == 0) ? 0 : 1;
			case 'n':
				return (strlen (val) != 0) ? 0 : 1;
			case 'e':
			case 'f':
			case 'd':
			case 's':
				return test_file (op, val);
			}
		return 1;
	}

	case 3: {
		const char* a = argv[0];
		const char* op = argv[1];
		const char* b = argv[2];

		if (strcmp (op, "=") == 0) return strcmp (a, b) == 0 ? 0 : 1;
		if (strcmp (op, "!=") == 0) return strcmp (a, b) != 0 ? 0 : 1;

		long ia = atol (a), ib = atol (b);
		if (strcmp (op, "-eq") == 0) return ia == ib ? 0 : 1;
		if (strcmp (op, "-ne") == 0) return ia != ib ? 0 : 1;
		if (strcmp (op, "-lt") == 0) return ia < ib ? 0 : 1;
		if (strcmp (op, "-le") == 0) return ia <= ib ? 0 : 1;
		if (strcmp (op, "-gt") == 0) return ia > ib ? 0 : 1;
		if (strcmp (op, "-ge") == 0) return ia >= ib ? 0 : 1;

		if (strcmp (a, "!") == 0) {
			char* sub[3] = {(char*)op, (char*)b, nullptr};
			return !builtin_test (2, sub - 1);
		}
		return 1;
	}

	default:
		return 1;
	}
}