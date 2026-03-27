#ifndef STDIO_H
#define STDIO_H

#include <kernel/console.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void ksprintf (char* buf, const char* fmt, ...);
void kprintf (const char*, ...);

void kserial_printf (const char* fmt, ...);

#endif