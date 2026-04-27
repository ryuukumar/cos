#pragma once

#include <stdint.h>

void kernel_execve_as_user (const char* path, char* const argv[], char* const envp[]);

uint64_t sys_execve (uint64_t path, uint64_t argv, uint64_t envp);
