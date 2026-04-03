#include <kernel/io.h>
#include <utils/spinlock.h>

inline uint64_t spinlock_acquire (bool* lock) {
	uint64_t flags = save_irq_disable ();
	while (__atomic_test_and_set (lock, __ATOMIC_ACQUIRE))
		__asm__ volatile ("pause");
	return flags;
}

inline void spinlock_release (bool* lock, uint64_t flags) {
	__atomic_clear (lock, __ATOMIC_RELEASE);
	restore_irq (flags);
}