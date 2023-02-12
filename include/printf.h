#ifndef _PRINTF_H
#define _PRINTF_H
#include "a.h"
#include <stdint.h>

void vsprintf_helper(char * str, void (*putchar)(char), const char * format, uint32_t * pos, va_list arg);

void printf(const char * s, ...);

#endif