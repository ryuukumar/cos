#include <kclib/memory.h>
#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/console.h>
#include <kernel/serial.h>
#include <liballoc/liballoc.h>

/*!
 * Handle the next % case in printf. Currently handles: d,i -> signed int, u -> unsigned int, x,X ->
 * unsigned hexadecimal (case not considered), l,ll prefix -> 64-bit, h prefix -> 16-bit, hh prefix
 * -> 8-bit, s -> string, % -> actual % sign
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

	idx++;
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
		buf = kstrdup (va_arg (*list, const char*));
		break;
	case '%':
		buf = kmalloc (2);
		buf[0] = '%';
		buf[1] = '\0';
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

	*r_output = buf;
	return consumed;
}

static void kv_core_printf (const char* fmt, va_list* list, void (*putchar_call) (unsigned char)) {
	char* tmpbuf = nullptr;
	for (size_t input_idx = 0; input_idx < kstrlen (fmt);) {
		if (fmt[input_idx] == '%') {
			input_idx += format_handler (fmt, input_idx, list, &tmpbuf);
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
	kvprintf (fmt, &ap);
	va_end (ap);
}