#include "string.h"
#include <stdint.h>
#include "printf.h"

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

void *memset(void *dst,char val, int n)
{
    char *temp = dst;
    for(;n != 0; n--) *temp++ = val;
    return dst;
}

void *memcpy(void *dst, void const *src, int n)
{
    char * ret = dst;
    char * p = dst;
    const char * q = src;
    while (n--)
        *p++ = *q++;
    return ret;
}

uint16_t *memsetw(uint16_t *dest, uint16_t val, uint32_t count)
{
    uint16_t *temp = (uint16_t *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

int isspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

int strcpy(char *dst,const char *src)
{
    int i = 0;
    while ((*dst++ = *src++) != 0)
        i++;
    return i;
}

void itoa(char *buf, unsigned long int n, int base)
{
    unsigned long int tmp;
    int i, j;

    tmp = n;
    i = 0;

    do {
        tmp = n % base;
        buf[i++] = (tmp < 10) ? (tmp + '0') : (tmp + 'a' - 10);
    } while (n /= base);
    buf[i--] = 0;

    for (j = 0; j < i; j++, i--) {
        tmp = buf[j];
        buf[j] = buf[i];
        buf[i] = tmp;
    }
}

int atoi(char * string) {
    int result = 0;
    unsigned int digit;
    int sign;

    while (isspace(*string)) {
        string += 1;
    }

    /*
     * Check for a sign.
     */

    if (*string == '-') {
        sign = 1;
        string += 1;
    } else {
        sign = 0;
        if (*string == '+') {
            string += 1;
        }
    }

    for ( ; ; string += 1) {
        digit = *string - '0';
        if (digit > 9) {
            break;
        }
        result = (10*result) + digit;
    }

    if (sign) {
        return -result;
    }
    return result;
}

void print_to_none(void)
{

}

void sprintf(char *buf, char *a, ...)
{
    va_list ap;
    va_start(ap, a);
    vsprintf(buf, print_to_none, a, ap);
    va_end(ap);
}