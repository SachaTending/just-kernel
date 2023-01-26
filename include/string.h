#ifndef _STRING_H
#define _STRING_H
#include <stddef.h>
#include <stdint.h>

size_t strlen(const char* str);
int strcpy(char *dst,const char *src);

void *memset(void *dst,char val, int n);
uint16_t *memsetw(uint16_t *dest, uint16_t val, uint32_t count);

void *memcpy(void *dst, void const *src, int n);

int atoi(char * string);
void itoa(char *buf, unsigned long int n, int base);

#endif