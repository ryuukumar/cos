#pragma once

uint64_t spinlock_acquire (bool* lock);
void	 spinlock_release (bool* lock, uint64_t flags);
