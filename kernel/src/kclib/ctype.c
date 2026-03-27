#include <kclib/ctype.h>

/*!
 * Checks if a character is alphanumeric
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
inline int isalnum (char c) { return isalpha (c) || isdigit (c); }

/*!
 * Checks if a character is alphabetic
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
inline int isalpha (char c) { return islower (c) || isupper (c); }

/*!
 * Checks if a character is lowercase
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
inline int islower (char c) { return 'a' <= c && c <= 'z'; }

/*!
 * Checks if a character is an uppercase character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
inline int isupper (char c) { return 'A' <= c && c <= 'Z'; }

/*!
 * Checks if a character is a digit
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
inline int isdigit (char c) { return '0' <= c && c <= '9'; }

/*!
 * Checks if a character is a hexadecimal character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
inline int isxdigit (char c) {
	return isdigit (c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
}

/*!
 * Checks if a character is a control character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
inline int iscntrl (char c) { return c == (char)127 || ((char)0 <= c && c <= (char)31); }

/*!
 * Checks if a character is a graphical character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
inline int isgraph (char c) { return (char)33 <= c && c <= (char)126; }

/*!
 * Checks if a character is a space character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
inline int isspace (char c) { return c == ' ' || ((char)9 <= c && c <= (char)13); }

/*!
 * Checks if a character is a blank character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
inline int isblank (char c) { return c == '\t' || c == ' '; }

/*!
 * Checks if a character is a printing character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
inline int isprint (char c) { return isgraph (c) || c == ' '; }

/*!
 * Checks if a character is a punctuation character
 * @param c character to assess
 * @return non-zero if matches, else 0
 */
inline int ispunct (char c) { return isgraph (c) && !isalnum (c); }

/*!
 * Converts a character to lowercase
 * @param c character to assess
 * @return lowercase character if matches, else c
 */
inline char tolower (char c) { return isupper (c) ? (c + ('a' - 'A')) : c; }

/*!
 * Converts a character to uppercase
 * @param c character to assess
 * @return uppercase character if matches, else c
 */
inline char toupper (char c) { return islower (c) ? (c - ('a' - 'A')) : c; }