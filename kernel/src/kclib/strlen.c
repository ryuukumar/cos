#include <kclib/string.h>

/*!
 * Get the length of a standard string (terminating with 0).
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
 * Get the length of a standard string (terminating with 0). Stops search at strsz, if 0 character
 * is not yet encountered.
 * @param s string to check
 * @param strsz max length to scan
 * @return min of string size and strsz
 */
size_t kstrlen_s (const char* s, size_t strsz) {
	size_t ret = 0;
	while (ret < strsz && s[ret] != 0)
		ret++;
	return ret;
}