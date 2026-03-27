#include <kclib/string.h>

/*!
 * Returns the length of the given null-terminated byte string, that is, the number of characters in
 * a character array whose first element is pointed to by s up to and not including the first null
 * character.
 * @param s string to check
 * @return string size
 */
size_t kstrlen (const char* s) {
	size_t ret = 0;
	while (s[ret] != 0)
		ret++;
	return ret;
}

/*!
 * Returns the length of the given null-terminated byte string, that is, the number of characters in
 * a character array whose first element is pointed to by s up to and not including the first null
 * character.
 *
 * Returns n if the null character was not found in the first n bytes of s.
 *
 * @param s string to check
 * @param n max length to scan
 * @return min of string size and strsz
 */
size_t kstrnlen (const char* s, size_t n) {
	size_t ret = 0;
	while (ret < n && s[ret] != 0)
		ret++;
	return ret;
}