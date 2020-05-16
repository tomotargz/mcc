#include <stdlib.h>

#include "node.h"

Node* newNode(NodeKind kind, Node* lhs, Node* rhs)
{
    Node* node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node* newNodeNum(int val)
{
    Node* node = calloc(1, sizeof(Node));
    node->kind = NODE_NUMBER;
    node->val = val;
    return node;
}

Node* newNodeLocalVariable(int offset)
{
    Node* node = calloc(1, sizeof(Node));
    node->kind = NODE_LOCAL_VARIABLE;
    node->offset = offset;
    return node;
}
