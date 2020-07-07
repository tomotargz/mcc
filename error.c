#include "mcc.h"

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

void error_at(char* position, char* source, char* fileName, char* format, ...)
{
    char* lineStart = position;
    while (lineStart > source && *(lineStart - 1) != '\n') {
        lineStart--;
    }
    int lineOffset = position - lineStart;
    char* lineEnd = position;
    while (*lineEnd != '\n') {
        lineEnd++;
    }
    char* line = calloc(lineEnd - lineStart, sizeof(char));
    strncpy(line, lineStart, lineEnd - lineStart);
    int lineNum = 1;
    for (char* rp = source; rp != position; rp++) {
        if (*rp == '\n') {
            lineNum++;
        }
    }
    fprintf(stderr, "%s:%d\n", fileName, lineNum);
    fprintf(stderr, "%s\n", line);
    fprintf(stderr, "%*s", lineOffset, "");
    fprintf(stderr, "^ ");
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    exit(1);
}
