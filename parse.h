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
    Variable* localVariables;
    int stackSize;
};

typedef struct {
    Variable* globalVariables;
    Function* functions;
} Program;

Program* parse(Token* tokens);

#endif
