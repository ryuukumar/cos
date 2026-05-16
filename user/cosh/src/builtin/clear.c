#include <builtin.h>
#include <stdio.h>

int builtin_clear (int argc, char** argv) {
	(void)argc;
	(void)argv;
	for (int i = 0; i < 60; i++)
		putchar ('\n');
	printf ("\033[1;1H");
	return 0;
}
