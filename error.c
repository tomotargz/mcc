#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void info(char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
}

void error(char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char* position, char* source, char* format, ...)
{
    va_list ap;
    va_start(ap, format);

    fprintf(stderr, "%s\n", source);
    fprintf(stderr, "%*s", (int)(position - source), "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    exit(1);
}
