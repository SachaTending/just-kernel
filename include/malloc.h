#ifndef _MALLOC_H
#define _MALLOC_H
#include "stdint.h"
#include "stddef.h"

void mm_init(uint32_t kernel_end);
char* malloc(size_t size);
void free(void *mem);
void mm_print_out();

#endif