#ifndef PARSE_H
#define PARSE_H

#include "node.h"
#include "tokenize.h"

typedef struct LVar LVar;
struct LVar {
    char* name;
    int offset;
    LVar* next;
};

typedef struct Function Function;
struct Function{
    Function* next;
    char* name;
    Node* node;
    LVar* lVars;
    int stackSize;
};

Function* parse(Token* tokens);

#endif
