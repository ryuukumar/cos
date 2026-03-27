#include <kclib/string.h>

/*!
 * A sequence of calls to kstrtok_r breaks the string pointed to by s1 into a sequence of tokens,
 * each of which is delimited by a character from the string pointed to by s2. Each call in the
 * sequence has a search target:
 *
 *  - If s1 is non-null, the call is the first call in the sequence. The search target is the
 * null-terminated byte string pointed to by s1.
 *
 *  - If s1 is null, the call is one of the subsequent calls in the sequence. The search target is
 * determined by the previous call in the sequence (stored in *saveptr).
 *
 * Each call searches the search target for the first character that is not contained in the
 * separator string pointed to by s2. The separator string can be different from call to call.
 *
 *  - If no such character is found, there are no tokens in the search target. The search target for
 * the next call is unchanged.
 *
 *  - If such a character is found, it is the start of the current token. The function then searches
 * from there for the first character that is contained in the separator string.
 *
 *  - If no such character is found, the current token extends to the end of the search target.
 * The search target for the next call is set to an empty string.
 *
 *  - If such a character is found, it is overwritten by a null character, which terminates the
 * current token. The search target for the next call starts from the following character.
 *
 * @param s1 the string to tokenise, or nullptr to continue tokenising the same string.
 * @param s2 a string containing the delimiter characters.
 * @param saveptr a pointer to a char* that holds the context between calls.
 * @return a pointer to the next token, or nullptr if no more tokens are found.
 */
char* kstrtok_r (char* restrict s1, const char* restrict s2, char** restrict saveptr) {
	if (s1 == nullptr) {
		s1 = *saveptr;
		if (s1 == nullptr) return nullptr;
	}

	while (*s1 && kstrchr ((char*)s2, *s1) != nullptr)
		s1++;

	if (*s1 == '\0') {
		*saveptr = nullptr;
		return nullptr;
	}

	char* token_start = s1;

	while (*s1 && kstrchr ((char*)s2, *s1) == nullptr)
		s1++;

	if (*s1 == '\0') {
		*saveptr = nullptr;
	} else {
		*s1 = '\0';
		*saveptr = s1 + 1;
	}

	return token_start;
}