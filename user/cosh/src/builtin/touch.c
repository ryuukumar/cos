#include <builtin.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int builtin_touch (int argc, char** argv) {
	if (argc == 1) {
		printf ("usage: touch file");
		return -127;
	}

	int fd = open (argv[1], O_CREAT | O_WRONLY, 0644);
	if (fd < 0) {
		perror (argv[1]);
		return 1;
	}
	close (fd);
	return 0;
}
