#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

void ksprintf (char* buf, const char* fmt, ...);
void kprintf (const char*, ...);

void kserial_printf (const char* fmt, ...);
