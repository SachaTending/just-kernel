#include "math.h"

f64 fabs(f64 x) {
    return x < 0.0 ? -x : x;
}

f64 fmod(f64 x, f64 m) {
    f64 result;
    asm("1: fprem\n\t"
        "fnstsw %%ax\n\t"
        "sahf\n\t"
        "jp 1b"
        : "=t"(result) : "0"(x), "u"(m) : "ax", "cc");
    return result;
}

f64 sin(f64 x) {
    f64 result;
    asm("fsin" : "=t"(result) : "0"(x));
    return result;
}

f64 cos(f64 x) {
    return sin(x + PI / 2.0);
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

unsigned long map(unsigned long x, unsigned long in_min, unsigned long in_max, unsigned long out_min, unsigned long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}