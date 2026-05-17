#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>
#include <sys/stat.h>

int fstat (int file, struct stat* st) {
	return (int)syscall_ret ((long)syscall2 (SYSCALL_SYS_FSTAT, (uint64_t)file, (uint64_t)st));
}

int stat (const char* restrict path, struct stat* restrict st) {
	return (int)syscall_ret ((long)syscall2 (SYSCALL_SYS_STAT, (uint64_t)path, (uint64_t)st));
}