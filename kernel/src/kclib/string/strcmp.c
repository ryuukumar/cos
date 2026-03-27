#include <kclib/string.h>

/*!
 * Compares two null-terminated byte strings lexicographically.
 * The sign of the result is the sign of the difference between the values of the first pair of
 * characters (both interpreted as unsigned char) that differ in the strings being compared.
 * @param s1 first string
 * @param s2 second string
 * @return negative if s1 appears earlier lexographically, positive if s2 appears earlier
 * lexographically, 0 if strings equal (respects limit)
 */
int kstrcmp (const char* s1, const char* s2) {
	for (size_t i = 0;; i++) {
		if (s1[i] != s2[i]) return (int)(unsigned char)s1[i] - (int)(unsigned char)s2[i];
		if (s1[i] == 0 && s2[i] == 0) break;
	}
	return 0;
}

/*!
 * Compares at most count characters of two possibly null-terminated arrays. The comparison is done
 * lexicographically. Characters following the null character are not compared.
 * The sign of the result is the sign of the difference between the values of the first pair of
 * characters (both interpreted as unsigned char) that differ in the arrays being compared.
 * @param s1 first string
 * @param s2 second string
 * @param n limit on number of characters to compare
 * @return negative if s1 appears earlier lexographically, positive if s2 appears earlier
 * lexographically, 0 if strings equal (respects limit)
 */
int kstrncmp (const char* s1, const char* s2, size_t n) {
	for (size_t i = 0; i < n; i++) {
		if (s1[i] != s2[i]) return (int)(unsigned char)s1[i] - (int)(unsigned char)s2[i];
		if (s1[i] == 0 && s2[i] == 0) break;
	}
	return 0;
}