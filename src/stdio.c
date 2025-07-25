#include <kernel/console.h>
#include <stdio.h>
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