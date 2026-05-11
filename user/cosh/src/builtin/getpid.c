#include <builtin.h>
#include <stdio.h>
#include <unistd.h>

int builtin_getpid (int argc, char** argv) {
	(void)argc, (void)argv;
	pid_t pid = getpid ();
	if (pid >= 1) {
		printf ("%d\n", pid);
		return 0;
	}
	return pid;
}
