#ifndef PARSE_H
#define PARSE_H

#include "type.h"
#include "localVariable.h"
#include "node.h"
#include "tokenize.h"

typedef struct Function Function;
struct Function {
    Function* next;
    char* name;
    Node* node;
    Node* params;
    LocalVariable* localVariables;
    int stackSize;
};

Function* parse(Token* tokens);

#endif
