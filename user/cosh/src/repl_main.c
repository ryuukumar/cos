#include <dispatch.h>
#include <gen_argv.h>
#include <repl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char* pathbuf = nullptr;
char* cmdbuf = nullptr;

static int last_exit = 0;

static char* expand_vars (const char* src) {
	char* out = malloc (1000);
	char* dst = out;
	while (*src) {
		if (*src == '$') {
			src++;
			if (*src == '?') {
				dst += snprintf (dst, 32, "%d", last_exit);
				src++;
			} else if (*src == '$') {
				dst += snprintf (dst, 32, "%d", (int)getpid ());
				src++;
			} else {
				*dst++ = '$';
			}
		} else {
			*dst++ = *src++;
		}
	}
	*dst = '\0';
	return out;
}

static void strip_comment (char* buf) {
	bool in_single = false, in_double = false;
	for (char* p = buf; *p; p++) {
		if (*p == '\'' && !in_double) in_single = !in_single;
		if (*p == '"' && !in_single) in_double = !in_double;
		if (*p == '#' && !in_single && !in_double) {
			*p = '\0';
			break;
		}
	}
}

int run_line (char* line) {
	int	  last = 0;
	char* p = line;
	char* seg = p;
	bool  in_single = false, in_double = false;

	for (;;) {
		if (*p == '\'' && !in_double) in_single = !in_single;
		if (*p == '"' && !in_single) in_double = !in_double;

		if ((*p == ';' && !in_single && !in_double) || *p == '\0') {
			bool done = (*p == '\0');
			*p = '\0';

			while (*seg == ' ' || *seg == '\t')
				seg++;

			if (*seg != '\0') {
				char*  expanded = expand_vars (seg);
				size_t argc = 0;
				char** argv = gen_argv (expanded, &argc);
				last = dispatch (argc, argv);
				free (expanded);
			}

			if (done) break;
			seg = ++p;
		} else {
			p++;
		}
	}
	return last;
}

int repl_loop (void) {
	if (!pathbuf || !cmdbuf) return -1;

	getcwd (pathbuf, 100);
	printf ("root@cos %s > ", pathbuf);

	if (fgets (cmdbuf, 1000, stdin) == nullptr) return -1;

	cmdbuf[strcspn (cmdbuf, "\n")] = '\0';
	if (cmdbuf[0] == '\0') return 0;
	strip_comment (cmdbuf);
	last_exit = run_line (cmdbuf);
	if (last_exit != 0) printf ("exited with non-zero status: %i\n", last_exit);

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
