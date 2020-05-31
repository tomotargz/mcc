#ifndef VARIABLE_H
#define VARIABLE_H

#include "type.h"

typedef struct Variable Variable;
struct Variable {
    char* name;
    int offset;
    Type* type;
    Variable* next;
};

#endif
