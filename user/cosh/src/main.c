#include <builtin.h>
#include <repl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main (int argc, char** argv) {
	if (argc >= 2) {
		int ret = builtin_source (argc, argv);
		exit (ret);
	}
	repl ();
	exit (0);
}