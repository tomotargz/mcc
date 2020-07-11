#include <stdarg.h>

int sum1(int a, ...)
{
    int sum = a;
    va_list ap;
    va_start(ap, a);
    while (1) {
        int n = va_arg(ap, int);
        if (n == 0) {
            return sum;
        }
        sum += n;
    }
}

int sum2(int a, int b, ...)
{
    int sum = a + b;
    va_list ap;
    va_start(ap, b);
    while (1) {
        int n = va_arg(ap, int);
        if (n == 0) {
            return sum;
        }
        sum += n;
    }
}
