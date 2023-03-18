#pragma once
long map(long x, long in_min, long in_max, long out_min, long out_max);
unsigned long map(unsigned long x, unsigned long in_min, unsigned long in_max, unsigned long out_min, unsigned long out_max);

typedef double f64;

#define E 2.71828
#define PI 3.14159265358979323846264338327950
f64 fabs(f64 x);
f64 fmod(f64 x, f64 m);
f64 sin(f64 x);
f64 cos(f64 x);