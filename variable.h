#ifndef VARIABLE_H
#define VARIABLE_H

#include <stdbool.h>
#include "type.h"

typedef struct Variable Variable;
struct Variable {
    char* name;
    int offset;
    Type* type;
    bool isGlobal;
    Variable* next;
};

#endif
