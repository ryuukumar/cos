#include <kclib/string.h>
#include <kernel/error.h>

/*!
 * Copies the null-terminated byte string pointed to by s2, including the null terminator, to the
 * character array whose first element is pointed to by s1.
 * @param s1 destination string
 * @param s2 source string
 * @return s1
 */
char* kstrcpy (char* restrict s1, const char* restrict s2) {
	size_t len = kstrlen (s2);
	for (size_t i = 0; i <= len; i++)
		s1[i] = s2[i];
	return s1;
}

/*!
 * Copies at most n characters of the character array pointed to by s2 (including the terminating
 * null character, but not any of the characters that follow the null character) to character array
 * pointed to by s1.
 *
 * If n is reached before the entire array s2 was copied, the resulting character array is not
 * null-terminated.
 *
 * If, after copying the terminating null character from s2, n is not reached, additional null
 * characters are written to s1 until the total of n characters have been written.
 *
 * @param s1 destination string
 * @param s2 source string
 * @return s1
 */
char* kstrncpy (char* restrict s1, const char* restrict s2, size_t n) {
	size_t len = kstrnlen (s2, n), i = 0;
	for (; i < len; i++)
		s1[i] = s2[i];
	for (; i < n; i++)
		s1[i] = 0;
	return s1;
}
