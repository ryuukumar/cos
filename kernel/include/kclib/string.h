#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void   reverse (char* str);
size_t strlen (const char*);
void   itos (int32_t, char*, uint32_t);
void   ulitos (uint64_t, char*, uint32_t);
int	   strcmp (const char* a, const char* b);
char*  strdup (const char* s);

#endif