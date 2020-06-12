#ifndef VARIABLE_H
#define VARIABLE_H

#include "type.h"
#include <stdbool.h>

typedef struct ValueList {
    struct ValueList* next;
    int value;
} ValueList;

typedef struct InitialValue {
    ValueList* valueList;
    char* label;
    char* string;
} InitialValue;

typedef struct Variable Variable;
struct Variable {
    char* name;
    int offset;
    Type* type;
    bool isGlobal;
    char* string;
    InitialValue* initialValue;
};

typedef struct VariableList {
    struct VariableList* next;
    Variable* variable;
} VariableList;

#endif
