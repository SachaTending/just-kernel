typedef double f64;

#define E 2.71828
#define PI 3.14159265358979323846264338327950

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
