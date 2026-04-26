#pragma once

#include <kernel/memmgt.h>

paddr_t alloc_ppages (uint64_t count);
paddr_t alloc_ppage (void);
void	free_ppages (void* paddr, uint64_t count);
void	free_ppage (void* paddr);
void	init_physical_bitmap (struct limine_memmap_response* memmap_response);
