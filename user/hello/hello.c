/*
 * hello.c
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main (int argc, char* argv[]) {
	(void)argc, (void)argv;
	printf ("/bin/hello started.\n");

	int pid = fork ();
	if (pid == 0) {
		char* argv_pass[] = {"/bin/cosh", nullptr};
		char* envp_pass[] = {"PATH=/bin", nullptr};
		execve ("/bin/cosh", argv_pass, envp_pass);
	} else {
		int exit_stat = 0;
		waitpid (pid, &exit_stat, 0);

		printf ("/bin/cosh exited with %x\n", exit_stat);
	}
	return 0;
}