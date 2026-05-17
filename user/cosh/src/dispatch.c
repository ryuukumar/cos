#include <builtin.h>
#include <dispatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef WCOREDUMP
#define WCOREDUMP(s) ((s) & 0x80)
#endif

extern char** environ;

static void print_signal_status (int status) {
	if (!WIFSIGNALED (status)) return;
	fprintf (stderr, "%s%s\n", strsignal (WTERMSIG (status)),
			 WCOREDUMP (status) ? " (core dumped)" : "");
}

static int exit_status_from_wait (int status) {
	if (WIFSIGNALED (status)) return 128 + WTERMSIG (status);
	return WEXITSTATUS (status);
}

static int exec_and_wait (const char* path, char** argv) {
	pid_t pid = fork ();
	if (pid < 0) {
		perror ("fork");
		return -1;
	}
	if (pid == 0) {
		execve (path, argv, environ);
		_exit (127);
	}
	int status = 0;
	waitpid (pid, &status, 0);
	print_signal_status (status);
	return exit_status_from_wait (status);
}

static int try_exec_in_path (char** argv) {
	const char* PATH = getenv ("PATH");
	if (!PATH) return -1;

	char* path_copy = strdup (PATH);
	char* entry = strtok (path_copy, ":");
	while (entry) {
		size_t len = strlen (entry) + 1 + strlen (argv[0]) + 1;
		char*  buf = malloc (len);
		snprintf (buf, len, "%s/%s", entry, argv[0]);

		pid_t pid = fork ();
		if (pid == 0) {
			execve (buf, argv, environ);
			_exit (127);
		}
		int status = 0;
		waitpid (pid, &status, 0);
		free (buf);

		if (exit_status_from_wait (status) != 127) {
			free (path_copy);
			print_signal_status (status);
			return exit_status_from_wait (status);
		}
		entry = strtok (NULL, ":");
	}
	free (path_copy);
	return -1;
}

int dispatch (size_t argc, char** argv) {
	if (argc == 0) return 0;

	int d_builtin = dispatch_builtin (argc, argv);
	if (d_builtin != -1) return d_builtin;

	int ret = exec_and_wait (argv[0], argv);
	if (ret != 127) return ret;

	ret = try_exec_in_path (argv);
	if (ret != -1) return ret;

	printf ("cosh: command not found: %s\n", argv[0]);
	return -127;
}