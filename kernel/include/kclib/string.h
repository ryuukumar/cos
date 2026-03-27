#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void   kreverse (char* str);
size_t kstrlen (const char*);
void   kitos (int32_t, char*, uint32_t);
void   kulitos (uint64_t, char*, uint32_t);
int	   kstrcmp (const char* a, const char* b);
char*  kstrdup (const char* s);

#endif