#include <builtin.h>
#include <unistd.h>

int builtin_chdir (int argc, char** argv) {
	if (argc == 1)
		return chdir ("/");
	else
		return chdir (argv[1]);
}