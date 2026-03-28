#include <kclib/string.h>

/*!
 * Appends a copy of the null-terminated byte string pointed to by s2 to the end of the
 * null-terminated byte string pointed to by s1. The character s2[0] replaces the null terminator at
 * the end of s1. The resulting byte string is null-terminated.
 * @param s1 destination string
 * @param s2 source string
 * @return s1
 */
char* kstrcat (char* restrict s1, const char* restrict s2) {
	size_t len = kstrlen (s1);
	kstrcpy (&s1[len], s2);
	return s1;
}

/*!
 * Appends at most n characters from the character array pointed to by s2, stopping if the null
 * character is found, to the end of the null-terminated byte string pointed to by s1. The character
 * s2[0] replaces the null terminator at the end of s1. The terminating null character is always
 * appended in the end (so the maximum number of bytes the function may write is n+1).
 * @param s1 destination string
 * @param s2 source string
 * @param n maximum length not inclusive of 0-terminator
 * @return s1
 */
char* kstrncat (char* restrict s1, const char* restrict s2, size_t n) {
	size_t len1 = kstrlen (s1);
	size_t len2 = kstrnlen (s2, n);

	kstrncpy (&s1[len1], s2, len2);
	s1[len1 + len2] = 0;

	return s1;
}