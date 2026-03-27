#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void*  kmemcpy (void* restrict s1, const void* restrict s2, size_t n);
void*  kmemccpy (void* restrict s1, const void* restrict s2, int c, size_t n);
void*  kmemmove (void* s1, const void* s2, size_t n);
char*  kstrcpy (char* restrict s1, const char* restrict s2);
char*  kstrncpy (char* restrict s1, const char* restrict s2, size_t n);
char*  kstrdup (const char* s);
char*  kstrndup (const char* s, size_t n);
char*  kstrcat (char* restrict s1, const char* restrict s2);
char*  kstrncat (char* restrict s1, const char* restrict s2, size_t n);
int	   kmemcmp (const void* s1, const void* s2, size_t n);
int	   kstrcmp (const char* s1, const char* s2);
int	   kstrcoll (const char* s1, const char* s2);
int	   kstrncmp (const char* s1, const char* s2, size_t n);
size_t kstrxfrm (char* restrict s1, const char* restrict s2, size_t n);
void*  kmemchr (void* s, int c, size_t n);
char*  kstrchr (char* s, int c);
size_t kstrcspn (const char* s1, const char* s2);
char*  kstrpbrk (char* s1, const char* s2);
char*  kstrrchr (char* s, int c);
size_t kstrspn (const char* s1, const char* s2);
char*  kstrstr (char* s1, const char* s2);
char*  kstrtok (char* restrict s1, const char* restrict s2);
void*  kmemset (void* s, int c, size_t n);
void*  kmemset_explicit (void* s, int c, size_t n);
char*  kstrerror (int errnum);
size_t kstrlen (const char* s);
size_t kstrnlen (const char* s, size_t n);

size_t kstrlen_s (const char* s, size_t strsz);
void*  kmemcpy_s (void* restrict s1, size_t s1max, const void* restrict s2, size_t n);
void*  kmemmove_s (void* s1, size_t s1max, const void* s2, size_t n);
char*  kstrcpy_s (char* restrict s1, size_t s1max, const char* restrict s2);
char*  kstrncpy_s (char* restrict s1, size_t s1max, const char* restrict s2, size_t n);
char*  kstrcat_s (char* restrict s1, size_t s1max, const char* restrict s2);
char*  kstrncat_s (char* restrict s1, size_t s1max, const char* restrict s2, size_t n);
char*  kstrtok_s (char* restrict s1, size_t* restrict s1max, const char* restrict s2,
				  char** restrict ptr);
void*  kmemset_s (void* s, size_t smax, int c, size_t n);
size_t kstrnlen_s (const char* s, size_t maxsize);

// TODO: these should be moved to some other lib

void kitos (int32_t i, char* buf, uint32_t b);
void kulitos (uint64_t i, char* buf, uint32_t b);

#endif