#include <builtin.h>
#include <repl.h>
#include <stdlib.h>
#include <string.h>

int builtin_eval (int argc, char** argv) {
	size_t len = 0;
	for (int i = 1; i < argc; i++)
		len += strlen (argv[i]) + 1;
	char* line = malloc (len + 1);
	line[0] = '\0';

	for (int i = 1; i < argc; i++) {
		strcat (line, argv[i]);
		if (i < argc - 1) strcat (line, " ");
	}

	int ret = run_line (line);
	free (line);
	return ret;
}
