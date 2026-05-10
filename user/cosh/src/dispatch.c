#include <builtin.h>
#include <dispatch.h>
#include <stdio.h>

int dispatch (size_t argc, char** argv) {
	if (argc == 0) return 0;

	int d_builtin = dispatch_builtin (argc, argv);
	if (d_builtin != -1) return d_builtin;

	printf ("cosh: command not found: %s\n", argv[0]);
	return -127;
}
