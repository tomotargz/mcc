#ifndef PARSE_H
#define PARSE_H

#include "localVariable.h"
#include "node.h"
#include "tokenize.h"
#include "type.h"

typedef struct Function Function;
struct Function {
    Function* next;
    char* name;
    Node* params;
    Node* statements;
    LocalVariable* localVariables;
    int stackSize;
};

Function* parse(Token* tokens);

#endif
