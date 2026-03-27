#include <kclib/string.h>
#include <kernel/error.h>

/*!
 * Copy a string from s2 to s1.
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
 * Copy a string from s2 to s1. Will limit to n characters, and does not guarantee s1 to be
 * 0-terminated. If s2 does not fill up n bytes in s1, the remaining bytes are set to 0.
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
