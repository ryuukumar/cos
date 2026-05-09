#include <repl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main (int argc, char* argv) {
	printf ("/bin/cosh started\n");
	repl ();
	exit (0);
}