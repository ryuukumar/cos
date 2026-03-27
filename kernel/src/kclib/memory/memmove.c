#include <kclib/string.h>

/*!
 * Copies n characters from the object pointed to by s2 to the object pointed to by s1. Both
 * objects are interpreted as arrays of unsigned char. The objects may overlap: copying takes place
 * as if the characters were copied to a temporary character array and then the characters were
 * copied from the array to s1.
 *
 * @param s1 destination buffer
 * @param s2 source buffer
 * @param n bytes to move
 * @return s1
 */
void* kmemmove (void* s1, const void* s2, size_t n) {
	unsigned char*		 pdest = (unsigned char*)s1;
	const unsigned char* psrc = (const unsigned char*)s2;

	if (s2 > s1)
		for (size_t i = 0; i < n; i++)
			pdest[i] = psrc[i];
	else if (s2 < s1)
		for (size_t i = n; i > 0; i--)
			pdest[i - 1] = psrc[i - 1];

	return s1;
}