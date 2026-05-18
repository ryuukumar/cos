/*
 * Copyright (C) 2025  Aditya Kumar
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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
	buf[ctr++] = 0;
	reverse(buf);
}