/*
 * dispatch_builtin.c
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
#include <string.h>

int dispatch_builtin (size_t argc, char** argv) {
	if (argc == 0) return -1;
	if (strcmp (argv[0], "exit") == 0)
		return builtin_exit (argc, argv);
	else if (strcmp (argv[0], "cd") == 0)
		return builtin_chdir (argc, argv);
	else if (strcmp (argv[0], "chdir") == 0)
		return builtin_chdir (argc, argv);
	else if (strcmp (argv[0], "pwd") == 0)
		return builtin_pwd (argc, argv);
	else if (strcmp (argv[0], "ls") == 0)
		return builtin_ls (argc, argv);
	else if (strcmp (argv[0], "true") == 0)
		return 1;
	else if (strcmp (argv[0], "false") == 0)
		return 0;
	else if (strcmp (argv[0], "echo") == 0)
		return builtin_echo (argc, argv);
	else if (strcmp (argv[0], "mkdir") == 0)
		return builtin_mkdir (argc, argv);
	else if (strcmp (argv[0], "stat") == 0)
		return builtin_stat (argc, argv);
	else if (strcmp (argv[0], "getpid") == 0)
		return builtin_getpid (argc, argv);
	else if (strcmp (argv[0], "touch") == 0)
		return builtin_touch (argc, argv);
	else if (strcmp (argv[0], "eval") == 0)
		return builtin_eval (argc, argv);
	else if (strcmp (argv[0], "source") == 0)
		return builtin_source (argc, argv);
	else if (strcmp (argv[0], "test") == 0 || strcmp (argv[0], "[") == 0)
		return builtin_test (argc, argv);
	else if (strcmp (argv[0], "clear") == 0)
		return builtin_clear (argc, argv);

	return -1;
}
