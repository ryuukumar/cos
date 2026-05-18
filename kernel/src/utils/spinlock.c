/*
 * spinlock.c
 * Copyright (C) 2026  Aditya Kumar
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, see <https://www.gnu.org/licenses/>.
 */

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