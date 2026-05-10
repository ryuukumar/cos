#include <builtin.h>
#include <string.h>

int dispatch_builtin (size_t argc, char** argv) {
	if (argc == 0) return -1;
	if (strcmp (argv[0], "exit") == 0) return builtin_exit (argc, argv);

	return -1;
}
