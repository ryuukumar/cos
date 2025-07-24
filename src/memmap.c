#include <stddef.h>
#include <stdint.h>
#include <memmgt.h>

__attribute__((section(".memory_map"), aligned(0x1000)))
struct {
    pdpt_entry_t pdpt_entry[512];
    pd_entry_t   pd_entry[512];
    pt_entry_t   pt_entry[512];
} memmap;