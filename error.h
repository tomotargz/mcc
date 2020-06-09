#ifndef ERROR_H
#define ERROR_H

void error(char* format, ...);
void error_at(char* position, char* source, char* fileName, char* format, ...);
void info(char* format, ...);

#endif
