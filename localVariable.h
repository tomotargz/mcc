#ifndef LOCAL_VARIABLE_H
#define LOCAL_VARIABLE_H

#include "type.h"

typedef struct Variable Variable;
struct Variable {
    char* name;
    int offset;
    Type* type;
    Variable* next;
};

#endif
