#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

void* memcpy (void*, const void*, size_t);
void* memset (void*, int, size_t);
void* memmove (void*, const void*, size_t);
int memcmp (const void*, const void*, size_t);

#endif