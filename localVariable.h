#ifndef LOCAL_VARIABLE_H
#define LOCAL_VARIABLE_H

#include "type.h"

typedef struct LocalVariable LocalVariable;
struct LocalVariable {
    char* name;
    int offset;
    Type* type;
    LocalVariable* next;
};

#endif
