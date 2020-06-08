#ifndef VARIABLE_H
#define VARIABLE_H

#include "type.h"
#include <stdbool.h>

typedef struct Variable Variable;
struct Variable {
    char* name;
    int offset;
    Type* type;
    bool isGlobal;
    char* string;
};

typedef struct VariableList {
    struct VariableList* next;
    Variable* variable;
} VariableList;

#endif
