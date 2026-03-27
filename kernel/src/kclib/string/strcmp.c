#include <kclib/string.h>

/*!
 * Compare two strings.
 * @param s1 first string
 * @param s2 second string
 * @return -1 if s1 appears earlier lexographically, 1 if s2 appears earlier lexographically, 0 if
 * strings equal (respects limit)
 */
int kstrcmp (const char* s1, const char* s2) {
	for (size_t i = 0;; i++) {
		if (s1[i] < s2[i]) return -1;
		if (s1[i] > s2[i]) return 1;
		if (s1[i] == 0 && s2[i] == 0) break;
	}
	return 0;
}

/*!
 * Compare two strings upto n characters.
 * @param s1 first string
 * @param s2 second string
 * @param n limit on number of characters to compare
 * @return -1 if s1 appears earlier lexographically, 1 if s2 appears earlier lexographically, 0 if
 * strings equal (respects limit)
 */
int kstrncmp (const char* s1, const char* s2, size_t n) {
	for (size_t i = 0; i < n; i++) {
		if (s1[i] < s2[i]) return -1;
		if (s1[i] > s2[i]) return 1;
		if (s1[i] == 0 && s2[i] == 0) break;
	}
	return 0;
}