
#include <stdio.h>
#include <unistd.h>

int main (int argc, char* argv[]) {
	printf ("Hello from USERLAND using newlib!!!\n");

	for (;;)
		;

	return 1;
}
