#include <kclib/string.h>

/*!
 * Compares the first count bytes of the objects pointed to by lhs and rhs. The comparison is done
 * lexicographically.
 *
 * The sign of the result is the sign of the difference between the values of the first pair of
 * bytes (both interpreted as unsigned char) that differ in the objects being compared.
 *
 * @param s1 first buffer to compare
 * @param s2 second buffer to compar
 * @param n number of bytes to compare between buffers
 * @return negative if s1 appears earlier lexographically, positive if s2 appears earlier
 * lexographically, 0 if buffers equal
 */
int kmemcmp (const void* s1, const void* s2, size_t n) {
	const unsigned char* p1 = (const unsigned char*)s1;
	const unsigned char* p2 = (const unsigned char*)s2;

	for (size_t i = 0; i < n; i++)
		if (p1[i] != p2[i]) return (int)p1[i] - (int)p2[i];

	return 0;
}