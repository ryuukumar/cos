#include <builtin.h>
#include <stdio.h>

int builtin_clear (int argc, char** argv) {
	(void)argc, (void)argv;
	printf ("\033[2J\033[3J\033[H");
	return 0;
}
