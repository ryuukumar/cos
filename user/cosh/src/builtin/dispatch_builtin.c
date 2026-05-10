#include <builtin.h>
#include <string.h>

int dispatch_builtin (size_t argc, char** argv) {
	if (argc == 0) return -1;
	if (strcmp (argv[0], "exit") == 0)
		return builtin_exit (argc, argv);
	else if (strcmp (argv[0], "cd") == 0)
		return builtin_chdir (argc, argv);
	else if (strcmp (argv[0], "chdir") == 0)
		return builtin_chdir (argc, argv);
	else if (strcmp (argv[0], "pwd") == 0)
		return builtin_pwd (argc, argv);
	else if (strcmp (argv[0], "ls") == 0)
		return builtin_ls (argc, argv);

	return -1;
}
