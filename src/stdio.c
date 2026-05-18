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

#include <stdio.h>
#include <console.h>
#include <string.h>


/*!
Handle the different cases of %-- in printf

@param	format the formatted string to be printed
@param	i	pointer to the index we are at
@param	args	pointer to the arguments printf received
@param	ul	whether the format prints an unsigned/long character
*/
void fmtprintf(const char* format, int* i, va_list* args, bool ul) {
	switch(format[*i]) {
		case 's':
		{
			const char* str = va_arg(*args, const char*);
			putstr(str, strlen(str));
		} break;
		case 'd':
		case 'i':
		{
			if (ul) {
				char buf [65] = {0};
				ulitos(va_arg(*args, uint64_t), buf, 10);
				putstr(buf, strlen(buf));
			}
			else {
				char buf [33] = {0};
				itos(va_arg(*args, int32_t), buf, 10);
				putstr(buf, strlen(buf));
			}
		} break;
		case 'x':
		{
			if (ul) {
				char buf [65] = {0};
				ulitos(va_arg(*args, uint64_t), buf, 16);
				putstr(buf, strlen(buf));
			}
			else {
				char buf [33] = {0};
				itos(va_arg(*args, int32_t), buf, 16);
				putstr(buf, strlen(buf));
			}
		} break;
		case 'u':
		case 'l':
		{
			(*i)++;
			fmtprintf(format, i, args, true);
		} break;
		default:
		{
			putchar('%');
			putchar(format[*i]);
		}
	}
}

/*!
Print a formatted string to the screen.

@param	format formatted string to print
*/
void printf(const char* format, ...) {
	va_list args;
	va_start (args, format);

	bool buf = get_update_on_putch();
	set_update_on_putch(false);
	for (int i=0; i<strlen(format); i++) {
		switch(format[i]) {
			case '%':
				{
					i++;
					fmtprintf(format, &i, &args, false);
				} break;
			case '\t':
				size_t idx = get_idx();
				idx = (idx / TAB_WIDTH + 1) * TAB_WIDTH;
				set_idx(idx);
				break;
			default:
				putchar(format[i]);
		}
	}
	update();
	set_update_on_putch(buf);
}