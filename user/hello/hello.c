#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main (int argc, char* argv[]) {
	(void)argc, (void)argv;
	printf ("/bin/hello started.\n");

	int pid = fork ();
	if (pid == 0) {
		char* argv_pass[] = {"/bin/cosh", nullptr};
		execv ("/bin/cosh", argv_pass);
	} else {
		int exit_stat = 0;
		waitpid (pid, &exit_stat, 0);

		printf ("/bin/cosh exited with %x\n", exit_stat);
	}
	return 0;
}