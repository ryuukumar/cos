#include <kclib/string.h>

/*!
 * Returns the length of the maximum initial segment of the null-terminated byte string pointed to
 * by s1, that consists of only the characters not found in the null-terminated byte string
 * pointed to by s2.
 *
 * @param s1 string to check in
 * @param s2 string which contains characters considered 'invalid'
 * @return the length of the maximum initial segment that contains only characters not found in the
 * null-terminated byte string pointed to by s2
 */
size_t kstrcspn (const char* s1, const char* s2) {
	bool   charmap[256] = {0};
	size_t len1 = kstrlen (s1), len2 = kstrlen (s2);
	for (size_t i = 0; i < len2; i++)
		charmap[(unsigned char)s2[i]] = true;
	for (size_t j = 0; j < len1; j++)
		if (charmap[(unsigned char)s1[j]]) return j;
	return len1;
}