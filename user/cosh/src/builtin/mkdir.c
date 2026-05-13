#include <builtin.h>
#include <stdio.h>
#include <sys/stat.h>

int builtin_mkdir (int argc, char** argv) {
	if (argc == 1) {
		printf ("usage: mkdir directory_name\n");
		return -127;
	}
	return mkdir (argv[1], 0755);
}
