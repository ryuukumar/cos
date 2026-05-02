#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void list_dir (const char* path) {
	printf ("\nListing: %s\n", path);
	DIR* dir = opendir (path);
	if (!dir) {
		perror ("opendir");
		return;
	}

	struct dirent* entry;
	while ((entry = readdir (dir)) != NULL) {
		const char* type = (entry->d_type == DT_DIR) ? "DIR" : "FILE";
		printf ("  [%s] %s\n", type, entry->d_name);
	}
	closedir (dir);
}

int main (int argc, char* argv[]) {
	printf ("Hello from USERLAND using newlib!!!\n");
	chdir ("usr/lib");
	list_dir ("..");
	list_dir ("/usr/lib");

	char* path = malloc (150);
	getcwd (path, 150);
	printf ("CWD: %s \n", path);
	return 0;
}