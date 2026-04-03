#ifndef SPINLOCK_H
#define SPINLOCK_H

uint64_t spinlock_acquire (bool* lock);
void	 spinlock_release (bool* lock, uint64_t flags);

#endif