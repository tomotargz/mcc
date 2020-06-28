#include "mcc.h"

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

void addType(Node* node)
{
    if (!node || node->type) {
        return;
    }

    addType(node->next);
    addType(node->lhs);
    addType(node->rhs);
    addType(node->cond);
    addType(node->then);
    addType(node->els);
    addType(node->body);
    addType(node->init);
    addType(node->inc);

    for (Node* statement = node->statements;
         statement;
         statement = statement->next) {
        addType(statement);
    }

    for (Node* arg = node->args; arg; arg = arg->next) {
        addType(arg);
    }

    if (node->kind == NODE_ADDITION
        || node->kind == NODE_SUBTRACTION
        || node->kind == NODE_MULTIPLICATION
        || node->kind == NODE_DIVISION
        || node->kind == NODE_EQUAL
        || node->kind == NODE_NOT_EQUAL
        || node->kind == NODE_LESS_THAN
        || node->kind == NODE_LESS_OR_EQUAL
        || node->kind == NODE_NUMBER) {
        node->type = &INT_TYPE;
        return;
    } else if (node->kind == NODE_POINTER_ADDITION
        || node->kind == NODE_POINTER_SUBTRACTION
        || node->kind == NODE_ASSIGNMENT
        || node->kind == NODE_PRE_INCREMENT
        || node->kind == NODE_POST_INCREMENT
        || node->kind == NODE_PRE_DECREMENT
        || node->kind == NODE_POST_DECREMENT) {
        node->type = node->lhs->type;
        return;
    } else if (node->kind == NODE_ADDR) {
        node->type = calloc(1, sizeof(Type));
        node->type->kind = TYPE_POINTER;
        node->type->pointerTo = node->lhs->type;
        return;
    } else if (node->kind == NODE_DEREF) {
        if (node->lhs->type->kind == TYPE_ARRAY) {
            node->type = node->lhs->type->arrayOf;
        } else if (node->lhs->type->kind == TYPE_POINTER) {
            node->type = node->lhs->type->pointerTo;
        }
        if (node->type == TYPE_VOID) {
            error("dereference to void type");
        }
        return;
    } else if (node->kind == NODE_MEMBER) {
        node->type = node->member->type;
    } else {
        node->type = &NO_TYPE;
        return;
    }
}
