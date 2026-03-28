#include <kclib/string.h>

/*!
 * Scans the null-terminated byte string pointed to by s1 for any character from the
 * null-terminated byte string pointed to by s2, and returns a pointer to that character.
 *
 * @param s1 string to be analysed
 * @param s2 string that contains the characters to search for
 * @return pointer to the first character in s1 that is also in s2, or nullptr if there is no such
 * character
 */
char* kstrpbrk (char* s1, const char* s2) {
	bool   charmap[256] = {0};
	size_t len1 = kstrlen (s1), len2 = kstrlen (s2);
	for (size_t i = 0; i < len2; i++)
		charmap[(unsigned char)s2[i]] = true;
	for (size_t j = 0; j < len1; j++)
		if (!charmap[(unsigned char)s1[j]]) return &s1[j];
	return nullptr;
}