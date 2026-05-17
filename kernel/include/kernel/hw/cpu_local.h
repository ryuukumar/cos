#pragma once
#include <stdint.h>

typedef struct {
	uint64_t user_rsp;
	uint64_t kernel_rsp;
} cpu_local_t;

extern cpu_local_t cpu_local;

void init_cpu_local (void);
