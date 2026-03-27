#include <kclib/string.h>
#include <liballoc/liballoc.h>

/*!
Reverse a standard string (terminating with 0).

@param  str string to reverse
*/
static void kreverse (char* str) {
	int len = kstrlen (str), start = 0, end = len - 1;
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
void kitos (int32_t i, char* buf, uint32_t b) {
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
	kreverse (buf);
}

/*!
Convert long integer to representative string with base b.

@param  i integer to convert
@param  buf memory to save integer
@param  b base
*/
void kulitos (uint64_t i, char* buf, uint32_t b) {
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
	kreverse (buf);
}

/*
 * Duplicate a string using the kernel allocator.
 * Returns a newly allocated copy or nullptr on failure.
 */
char* kstrdup (const char* s) {
	if (!s) return nullptr;
	size_t len = kstrlen (s);
	char*  dup = (char*)kmalloc (len + 1);
	if (!dup) return nullptr;
	for (size_t i = 0; i <= len; i++)
		dup[i] = s[i];
	return dup;
}
