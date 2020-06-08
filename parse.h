#ifndef PARSE_H
#define PARSE_H

#include "node.h"
#include "tokenize.h"
#include "type.h"
#include "variable.h"

typedef struct Function Function;
struct Function {
    Function* next;
    char* name;
    Node* params;
    Node* statements;
    VariableList* localVariables;
    int stackSize;
};

typedef struct {
    VariableList* globalVariables;
    Function* functions;
} Program;

Program* parse(Token* tokens);

#endif
