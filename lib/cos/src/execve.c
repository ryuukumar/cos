#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>
#include <stdint.h>

// Needed for linking
int _execve (const char* path, char* const argv[], char* const envp[]);
int _execve (const char* path, char* const argv[], char* const envp[]) {
	execve (path, argv, envp);
}

int execve (const char* path, char* const argv[], char* const envp[]) {
	return syscall3 (SYSCALL_SYS_EXECVE, (uint64_t)path, (uint64_t)argv, (uint64_t)envp);
}