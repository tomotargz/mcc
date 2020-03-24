#ifndef PARSE_H
#define PARSE_H

#include "node.h"
#include "tokenize.h"

typedef struct LVar LVar;
struct LVar {
    char* name;
    int len;
    int offset;
    LVar* next;
};

typedef struct {
    Node** ast;
    LVar* lvars;
} ParseResult;

ParseResult parse(Token* tokens);

#endif
