#include <stdlib.h>

#include "node.h"

Node* new_node(NodeKind kind, Node* lhs, Node* rhs)
{
    Node* node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node* new_node_num(int val)
{
    Node* node = calloc(1, sizeof(Node));
    node->kind = NODE_NUMBER;
    node->val = val;
    return node;
}
