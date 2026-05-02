#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>

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
	list_dir (".");
	list_dir ("/usr/lib");
	return 0;
}