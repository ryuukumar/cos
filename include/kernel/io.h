
#ifndef IO_H
#define IO_H

#include <stdint.h>

static inline uint8_t inb (uint16_t port);
static inline void outb (uint16_t port, uint8_t val);

#endif