#include <kclib/memory.h>
#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/console.h>
#include <kernel/serial.h>
#include <liballoc/liballoc.h>

static uint64_t uint_str_to_num (const char* str, size_t len) {
	uint64_t res = 0;
	for (size_t i = 0; i < len; i++) {
		res *= 10;
		res += str[i] - '0';
	}
	return res;
}

/*!
 * Handle the next % case in printf. Currently handles: d,i -> signed int, u -> unsigned int, x,X ->
 * unsigned hexadecimal (case not considered), l,ll prefix -> 64-bit, h prefix -> 16-bit, hh prefix
 * -> 8-bit, s -> string, % -> actual % sign, c -> character (signed), p -> 64-bit pointer
 * @param input full input fmt string
 * @param idx pointer to index at which % was encountered
 * @param list argument list
 * @param r_output pointer to char* which will be populated with output (will be mallocd)
 * @return number of bytes read from input
 */
static size_t format_handler (const char* input, size_t idx, va_list* list, char** r_output) {
	if (!input || input[idx] != '%') {
		*r_output = nullptr;
		return 0;
	}

	size_t consumed = 1; /* '%' */
	size_t input_bytes = 32;
	size_t pad_width = 0;
	char   pad_char = ' ';

	idx++;
	if ('0' <= input[idx] && input[idx] <= '9') {
		if (input[idx] == '0') {
			idx++, consumed++;
			pad_char = '0';
		}

		size_t num_len = idx;
		while ('0' <= input[idx] && input[idx] <= '9')
			idx++, consumed++;
		num_len = idx - num_len;

		pad_width = uint_str_to_num ((const char*)&input[idx - num_len], num_len);
	}

	while (input[idx] == 'l' || input[idx] == 'h') {
		if (input[idx] == 'l')
			input_bytes = 64;
		else
			input_bytes /= 2;
		idx++;
		consumed++;
	}

	char spec = input[idx++];
	consumed++;

	char* buf = NULL;
	switch (spec) {
	case 's':
		char* str = kstrdup (va_arg (*list, const char*));
		if (str) buf = str;
		break;
	case 'c':
		buf = kmalloc (2);
		buf[0] = (char)va_arg (*list, int);
		buf[1] = '\0';
		break;
	case '%':
		buf = kmalloc (2);
		buf[0] = '%';
		buf[1] = '\0';
		break;
	case 'p':
		buf = kmalloc (19);
		buf[0] = '0';
		buf[1] = 'x';
		kulitos ((uint64_t)va_arg (*list, void*), buf + 2, 16);
		break;
	case 'd':
	case 'i':
		if (input_bytes == 64) {
			buf = kmalloc (65);
			kulitos (va_arg (*list, uint64_t), buf, 10);
		} else {
			buf = kmalloc (33);
			kitos (va_arg (*list, int32_t), buf, 10);
		}
		break;
	case 'u':
		if (input_bytes == 64) {
			buf = kmalloc (65);
			kulitos (va_arg (*list, uint64_t), buf, 10);
		} else {
			buf = kmalloc (33);
			kulitos (va_arg (*list, uint32_t), buf, 10);
		}
		break;
	case 'x':
	case 'X':
		if (input_bytes == 64) {
			buf = kmalloc (17);
			kulitos (va_arg (*list, uint64_t), buf, 16);
		} else {
			buf = kmalloc (9);
			kulitos (va_arg (*list, uint32_t), buf, 16);
		}
		break;
	default:
		*r_output = nullptr;
		return consumed;
	}

	if (pad_width > 0 && buf) {
		size_t len = kstrlen (buf);

		if (pad_width > len) {
			size_t newlen = pad_width;
			char*  pbuf = kmalloc (newlen + 1);
			size_t pad = pad_width - len;

			for (size_t i = 0; i < pad; i++)
				pbuf[i] = pad_char;

			kmemcpy (pbuf + pad, buf, len + 1);
			kfree (buf);
			buf = pbuf;
		}
	}

	*r_output = buf;
	return consumed;
}

static void kv_core_printf (const char* fmt, va_list* list, void (*putchar_call) (unsigned char)) {
	char* tmpbuf = nullptr;
	for (size_t input_idx = 0; input_idx < kstrlen (fmt);) {
		if (fmt[input_idx] == '%') {
			input_idx += format_handler (fmt, input_idx, list, &tmpbuf);
			if (!tmpbuf) continue;
			for (size_t tmp_idx = 0; tmp_idx < kstrlen (tmpbuf); tmp_idx++)
				putchar_call (tmpbuf[tmp_idx]);
			kfree (tmpbuf), tmpbuf = nullptr;
		} else
			putchar_call (fmt[input_idx++]);
	}
}

static char*  kvsprintf_buf;
static size_t kvsprintf_idx;

static void kvsprintf_putc (unsigned char c) { kvsprintf_buf[kvsprintf_idx++] = c; }

static void kvsprintf (char* buf, const char* fmt, va_list* list) {
	kvsprintf_buf = buf;
	kvsprintf_idx = 0;
	kv_core_printf (fmt, list, kvsprintf_putc);
	buf[kvsprintf_idx] = '\0';
}

static void kvprintf (const char* fmt, va_list* list) {
	kv_core_printf (fmt, list, putchar);
	update ();
}

void ksprintf (char* buf, const char* fmt, ...) {
	va_list ap;
	va_start (ap, fmt);
	kvsprintf (buf, fmt, &ap);
	va_end (ap);
}

void kprintf (const char* fmt, ...) {
	va_list ap;
	va_start (ap, fmt);
	kvprintf (fmt, &ap);
	va_end (ap);
}

static void kvserial_printf (const char* fmt, va_list* list) {
	kv_core_printf (fmt, list, write_serial);
}

void kserial_printf (const char* fmt, ...) {
	va_list ap;
	va_start (ap, fmt);
	kvserial_printf (fmt, &ap);
	va_end (ap);
}