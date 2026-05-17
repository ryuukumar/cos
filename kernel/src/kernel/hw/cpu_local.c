#include <kernel/hw/cpu_local.h>
#include <kernel/io.h>

cpu_local_t cpu_local;

void init_cpu_local (void) {
	wrmsr (IA32_GS_BASE, 0);
	wrmsr (IA32_KERNEL_GS_BASE, (uint64_t)&cpu_local);
}
