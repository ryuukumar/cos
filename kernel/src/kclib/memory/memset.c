#include <kclib/string.h>

/*!
 * Copies the value (unsigned char)ch into each of the first count characters of the object pointed
 * to by dest.
 *
 * @param s pointer to the buffer to fill
 * @param c fill byte
 * @param n number of bytes to fill
 * @return s
 */
void* kmemset (void* s, int c, size_t n) {
	unsigned char* p = (unsigned char*)s;
	for (size_t i = 0; i < n; i++)
		p[i] = (unsigned char)c;
	return s;
}

/*!
 * Copies the value (unsigned char)ch into each of the first count characters of the object pointed
 * to by dest. Safe for sensitive information.
 *
 * @param s pointer to the buffer to fill
 * @param c fill byte
 * @param n number of bytes to fill
 * @return s
 */
void* kmemset_explicit (void* s, int c, size_t n) {
	volatile unsigned char* p = (volatile unsigned char*)s;
	for (size_t i = 0; i < n; i++)
		p[i] = (unsigned char)c;
	return s;
}