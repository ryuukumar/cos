#include <kclib/string.h>

/*!
 * Copies n characters from the object pointed to by s2 to the object pointed to by s1. Both
 * objects are interpreted as arrays of unsigned char.
 *
 * @param s1 destination buffer
 * @param s2 source buffer
 * @param n number of bytes to copy
 * @return s1
 */
void* kmemcpy (void* restrict s1, const void* restrict s2, size_t n) {
	unsigned char*		 pdest = (unsigned char*)s1;
	const unsigned char* psrc = (const unsigned char*)s2;
	for (size_t i = 0; i < n; i++)
		pdest[i] = psrc[i];
	return s1;
}