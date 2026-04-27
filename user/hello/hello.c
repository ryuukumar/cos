
#include <stdio.h>
#include <unistd.h>

int main (int argc, char* argv[]) {
	printf ("Hello from USERLAND using newlib!!!\n");

	char* new_argv[] = {"/bin/pishell", NULL};
	char* new_envp[] = {NULL};

	printf ("Attempting to execve /bin/pishell...\n");
	execve ("/bin/pishell", new_argv, new_envp);

	printf ("If you see this, execve failed!\n");
	return 1;
}
