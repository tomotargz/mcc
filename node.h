#ifndef NODE_H
#define NODE_H

#include "localVariable.h"
#include "type.h"

typedef enum {
    NODE_ADDITION,
    NODE_SUBTRACTION,
    NODE_POINTER_ADDITION,
    NODE_POINTER_SUBTRACTION,
    NODE_MULTIPLICATION,
    NODE_DIVISION,
    NODE_EQUAL,
    NODE_NOT_EQUAL,
    NODE_LESS_THAN,
    NODE_LESS_OR_EQUAL,
    NODE_NUMBER,
    NODE_ASSIGNMENT,
    NODE_LOCAL_VARIABLE,
    NODE_CONDITION,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_BLOCK,
    NODE_RETURN,
    NODE_CALL,
    NODE_ADDR,
    NODE_DEREF,
    NODE_NULL,
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node* next;
    Node* lhs;
    Node* rhs;
    int val;
    LocalVariable* localVariable;
    Type* type;

    // if(cond)then;else els;
    // while(cond)body;
    // for(init;cond;inc)body;
    Node* cond;
    Node* then;
    Node* els;
    Node* body;
    Node* init;
    Node* inc;

    Node* statements;

    // function
    char* name;
    Node* args;
};

Node* newNode(NodeKind kind, Node* lhs, Node* rhs);
Node* newNodeNum(int val);
Node* newNodeLocalVariable(LocalVariable* localVariable);
void addType(Node* tree);

#endif
