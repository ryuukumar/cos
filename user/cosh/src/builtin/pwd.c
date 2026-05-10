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