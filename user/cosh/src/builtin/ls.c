#include <builtin.h>
#include <dirent.h>
#include <stdio.h>

int builtin_ls (int argc, char** argv) {
	const char* path = (argc >= 2) ? argv[1] : ".";

	DIR* dir = opendir (path);
	if (!dir) {
		perror ("ls");
		return -1;
	}

	struct dirent* entry;
	while ((entry = readdir (dir)) != NULL)
		printf ("%s\n", entry->d_name);

	closedir (dir);
	return 0;
}