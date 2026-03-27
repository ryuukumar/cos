#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

void* kmemcpy (void*, const void*, size_t);
void* kmemset (void*, int, size_t);
void* kmemmove (void*, const void*, size_t);
int	  kmemcmp (const void*, const void*, size_t);

#endif