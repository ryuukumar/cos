#include <kclib/string.h>

/*!
 * Finds the first occurrence of (unsigned char)c in the initial n bytes (each interpreted as
 * unsigned char) of the object pointed to by s.
 *
 * @param s buffer to search in
 * @param c unsigned char to search for
 * @param n length of buffer to search in
 * @return pointer to the location of the byte, or a null pointer if no such byte is found.
 */
void* kmemchr (void* s, int c, size_t n) {
	unsigned char* p = (unsigned char*)s;
	for (size_t i = 0; i < n; i++)
		if (p[i] == (unsigned char)c) return (void*)(p + i);
	return nullptr;
}