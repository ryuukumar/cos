#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

size_t strlen (const char*);
void itos (int32_t, char*, uint32_t);
void ulitos (uint64_t, char*, uint32_t);

#endif STRING_H