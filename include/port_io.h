#ifndef _PORT_IO
#define _PORT_IO
#include <stdint.h>

void outportb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

uint16_t inw(uint16_t port);

#define inportb inb
#define inportw inw

#endif