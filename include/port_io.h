#ifndef _PORT_IO
#define _PORT_IO
#include <stdint.h>

void outportb(uint16_t port, uint8_t val);

uint8_t inb(uint16_t port);

void outportl(uint16_t portid, uint32_t value);

uint32_t inportl(uint16_t portid);

uint16_t inw(uint16_t port);

#define inportb inb
#define inportw inw

#endif