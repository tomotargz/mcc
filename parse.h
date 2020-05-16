#ifndef PARSE_H
#define PARSE_H

#include "node.h"
#include "tokenize.h"

typedef struct LocalVariable LocalVariable;
struct LocalVariable {
    char* name;
    int offset;
    LocalVariable* next;
};

typedef struct Function Function;
struct Function{
    Function* next;
    char* name;
    Node* node;
    Node* params;
    LocalVariable* localVariables;
    int stackSize;
};

Function* parse(Token* tokens);

#endif
