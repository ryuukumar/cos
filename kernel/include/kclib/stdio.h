#ifndef STDIO_H
#define STDIO_H

#include <kernel/console.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void kprintf (const char*, ...);

#endif