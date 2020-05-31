#include <stdlib.h>

#include "error.h"
#include "node.h"
#include "parse.h"
#include "type.h"

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

Node* newNodeVariable(Variable* variable)
{
    Node* node = calloc(1, sizeof(Node));
    node->kind = NODE_VARIABLE;
    node->variable = variable;
    return node;
}

void addType(Node* tree)
{
    if (!tree || tree->type) {
        return;
    }

    addType(tree->next);
    addType(tree->lhs);
    addType(tree->rhs);
    addType(tree->cond);
    addType(tree->then);
    addType(tree->els);
    addType(tree->body);
    addType(tree->init);
    addType(tree->inc);

    for (Node* statement = tree->statements;
         statement;
         statement = statement->next) {
        addType(statement);
    }

    for (Node* arg = tree->args; arg; arg = arg->next) {
        addType(arg);
    }

    static Type intType = { TYPE_INT, NULL };
    static Type pointerType = { TYPE_INT, NULL };
    static Type noType = { TYPE_NO, NULL };
    switch (tree->kind) {
    case NODE_ADDITION:
    case NODE_SUBTRACTION:
    case NODE_MULTIPLICATION:
    case NODE_DIVISION:
    case NODE_EQUAL:
    case NODE_NOT_EQUAL:
    case NODE_LESS_THAN:
    case NODE_LESS_OR_EQUAL:
    case NODE_NUMBER:
        tree->type = &intType;
        return;
    case NODE_POINTER_ADDITION:
    case NODE_POINTER_SUBTRACTION:
    case NODE_ASSIGNMENT:
        tree->type = tree->lhs->type;
        return;
    case NODE_VARIABLE:
        tree->type = tree->variable->type;
        return;
    case NODE_ADDR:
        tree->type = calloc(1, sizeof(Type));
        tree->type->kind = TYPE_POINTER;
        tree->type->pointerTo = tree->lhs->type;
        return;
    case NODE_DEREF:
        if (tree->lhs->type->kind == TYPE_ARRAY) {
            tree->type = tree->lhs->type->arrayOf;
            return;
        }
        tree->type = tree->lhs->type->pointerTo;
        return;
    default:
        tree->type = &noType;
        return;
    }
}
