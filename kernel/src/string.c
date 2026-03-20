#include <liballoc/liballoc.h>
#include <string.h>

/*!
Get the length of a standard string (terminating with 0).

@param  str pointer to string
@return string size
*/
size_t strlen (const char* str) {
	size_t ret = 0;
	while (str[ret] != 0)
		ret++;
	return ret;
}

/*!
Reverse a standard string (terminating with 0).

@param  str string to reverse
*/
void reverse (char* str) {
	int len = strlen (str), start = 0, end = len - 1;
	while (start < end) {
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		start++;
		end--;
	}
}

/*!
Convert integer to representative string with base b.

@param  i integer to convert
@param  buf memory to save integer
@param  b base
*/
void itos (int32_t i, char* buf, uint32_t b) {
	int	 ctr = 0;
	bool negative = false;
	if (i < 0) {
		i = -i;
		negative = true;
	}
	do {
		if (i % b < 10)
			buf[ctr++] = '0' + i % b;
		else
			buf[ctr++] = 'a' + i % b - 10;
		i /= b;
	} while (i);
	if (ctr == 0) buf[0] = '0';
	if (negative) buf[ctr++] = '-';
	buf[ctr + 1] = 0;
	reverse (buf);
}

/*!
Convert long integer to representative string with base b.

@param  i integer to convert
@param  buf memory to save integer
@param  b base
*/
void ulitos (uint64_t i, char* buf, uint32_t b) {
	int	 ctr = 0;
	bool negative = false;
	do {
		if (i % b < 10)
			buf[ctr++] = '0' + i % b;
		else
			buf[ctr++] = 'a' + i % b - 10;
		i /= b;
	} while (i);
	if (ctr == 0) buf[0] = '0';
	if (negative) buf[ctr++] = '-';
	buf[ctr++] = 0;
	reverse (buf);
}

/*!
 * Compare two strings.
 * @param a first string
 * @param b second string
 * @return whether strings are equal
 */
int strcmp (const char* a, const char* b) {
	for (int i = 0; a[i] != 0 && b[i] != 0; i++)
		if (a[i] != b[i]) return 1;
	return 0;
}

/*
 * Duplicate a string using the kernel allocator.
 * Returns a newly allocated copy or NULL on failure.
 */
char* strdup (const char* s) {
	if (!s) return NULL;
	size_t len = strlen (s);
	char*  dup = (char*)kmalloc (len + 1);
	if (!dup) return NULL;
	for (size_t i = 0; i <= len; i++)
		dup[i] = s[i];
	return dup;
}
