#include <repl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char* pathbuf = nullptr;
char* cmdbuf = nullptr;

int repl_loop (void) {
	if (!pathbuf || !cmdbuf) return -1;

	getcwd (pathbuf, 100);
	printf ("root@cos %s > ", pathbuf);

	if (fgets (cmdbuf, 1000, stdin) == nullptr) return -1;

	if (cmdbuf[0] == '\0') return 0;

	printf ("eval %s\n", cmdbuf);
	return 0;
}

int repl (void) {
	int exit_stat = 0;
	pathbuf = malloc (100);
	cmdbuf = malloc (1000);

	memset (pathbuf, 0, 100);
	memset (cmdbuf, 0, 1000);

	if (!pathbuf || !cmdbuf) return -1;

	do {
		exit_stat = repl_loop ();
	} while (exit_stat != -1);
	return -1;
}
