#include <builtin.h>
#include <stdlib.h>
#include <unistd.h>

int builtin_exit (int argc, char** argv) {
	if (argc == 1)
		exit (0);
	else
		exit (atoi (argv[1]));
}
