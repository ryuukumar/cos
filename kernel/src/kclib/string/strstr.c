#include <kclib/string.h>

/*!
 * Finds the first occurrence of the null-terminated byte string pointed to by s2 in the
 * null-terminated byte string pointed to by s1. The terminating null characters are not compared.
 *
 * @param s1 string to search in
 * @param s2 string to search for
 * @return pointer to the first character of the found substring in s1, or a null pointer if such
 * substring is not found. If s2 points to an empty string, s1 is returned.
 */
char* kstrstr (char* s1, const char* s2) {
	size_t len1 = kstrlen (s1), len2 = kstrlen (s2);
	if (len2 == 0) return s1;
	if (len1 < len2) return nullptr;

	for (size_t i = 0; i <= len1 - len2; i++)
		if (kstrncmp (&s1[i], s2, len2) == 0) return &s1[i];
	return nullptr;
}