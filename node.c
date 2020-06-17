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
    if (variable->isGlobal) {
        node->kind = NODE_GLOBAL_VARIABLE;
    } else {
        node->kind = NODE_LOCAL_VARIABLE;
    }
    node->name = variable->name;
    node->offset = variable->offset;
    node->type = variable->type;
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

    if (tree->kind == NODE_ADDITION
        || tree->kind == NODE_SUBTRACTION
        || tree->kind == NODE_MULTIPLICATION
        || tree->kind == NODE_DIVISION
        || tree->kind == NODE_EQUAL
        || tree->kind == NODE_NOT_EQUAL
        || tree->kind == NODE_LESS_THAN
        || tree->kind == NODE_LESS_OR_EQUAL
        || tree->kind == NODE_NUMBER) {
        tree->type = &INT_TYPE;
        return;
    } else if (tree->kind == NODE_POINTER_ADDITION
        || tree->kind == NODE_POINTER_SUBTRACTION
        || tree->kind == NODE_ASSIGNMENT) {
        tree->type = tree->lhs->type;
        return;
    } else if (tree->kind == NODE_ADDR) {
        tree->type = calloc(1, sizeof(Type));
        tree->type->kind = TYPE_POINTER;
        tree->type->pointerTo = tree->lhs->type;
        return;
    } else if (tree->kind == NODE_DEREF) {
        if (tree->lhs->type->kind == TYPE_ARRAY) {
            tree->type = tree->lhs->type->arrayOf;
            return;
        }
        tree->type = tree->lhs->type->pointerTo;
        return;
    } else if (tree->kind == NODE_MEMBER) {
        tree->type = tree->member->type;
    } else {
        tree->type = &NO_TYPE;
        return;
    }
}
