#include <kernel/memmgt.h>

bool is_locked = false;

int liballoc_lock (void) {
	is_locked = true;
	return 0;
}

int liballoc_unlock (void) {
	is_locked = false;
	return 0;
}

void* liballoc_alloc (size_t count) { return alloc_vpages (count, false); }

int liballoc_free (void* ptr, size_t count) {
	if (is_locked) return -7;

	free_vpages (ptr, count);
	return 0;
}