#include <builtin.h>
#include <stdio.h>

int builtin_echo (int argc, char** argv) {
	for (int i = 1; i < argc; i++)
		printf ("%s%s", argv[i], (i + 1 == argc) ? "" : " ");
	printf ("\n");
	return 0;
}
