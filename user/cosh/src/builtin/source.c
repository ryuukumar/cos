#include <builtin.h>
#include <repl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int builtin_source (int argc, char** argv) {
	if (argc < 2) {
		printf ("source: not enough arguments\n");
		return 1;
	}

	struct stat st;
	if (stat (argv[1], &st) != 0) {
		perror (argv[1]);
		return 1;
	}
	if (!S_ISREG (st.st_mode)) {
		printf ("source: %s: not a regular file\n", argv[1]);
		return 1;
	}

	FILE* f = fopen (argv[1], "r");
	if (!f) {
		perror ("source");
		return 1;
	}

	char* line = malloc (1000);
	int	  last = 0;
	while (fgets (line, 1000, f)) {
		line[strcspn (line, "\n")] = '\0';
		if (line[0] == '\0') continue;
		last = run_line (line);
	}

	free (line);
	fclose (f);
	return last;
}
