
#ifndef IO_H
#define IO_H

#include <stdint.h>

void	outb (uint16_t port, uint8_t val);
uint8_t inb (uint16_t port);
void	io_wait (void);

uint64_t save_irq_disable (void);
void	 restore_irq (uint64_t flags);

#endif