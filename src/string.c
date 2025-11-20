#include <string.h>

/*!
Get the length of a standard string (terminating with 0).

@param  str pointer to string
@return string size
*/
size_t strlen (const char* str) {
	size_t ret = 0;
	while (str[ret] != 0) ret++;
	return ret;
}

/*!
Reverse a standard string (terminating with 0).

@param  str string to reverse
*/
void reverse (char* str) {
	int len = strlen(str),
		start = 0,
		end = len - 1;
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
	int ctr = 0;
	bool negative = false;
	if (i<0) {
		i = -i;
		negative = true;
	}
	do {
		if (i%b < 10) buf[ctr++] = '0' + i%b;
		else buf[ctr++] = 'a' + i%b - 10;
		i/=b;
	} while(i);
	if (ctr == 0) buf[0] = '0';
	if (negative) buf[ctr++] = '-';
	buf[ctr+1] = 0;
	reverse(buf);
}

/*!
Convert long integer to representative string with base b.

@param  i integer to convert
@param  buf memory to save integer
@param  b base
*/
void ulitos (uint64_t i, char* buf, uint32_t b) {
	int ctr = 0;
	bool negative = false;
	do {
		if (i%b < 10) buf[ctr++] = '0' + i%b;
		else buf[ctr++] = 'a' + i%b - 10;
		i/=b;
	} while(i);
	if (ctr == 0) buf[0] = '0';
	if (negative) buf[ctr++] = '-';
	buf[ctr++] = 0;
	reverse(buf);
}