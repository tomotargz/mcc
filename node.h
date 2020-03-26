#ifndef NODE_H
#define NODE_H

typedef enum {
    NODE_ADDITION,
    NODE_SUBTRACTION,
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
    NODE_RETURN,
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node* lhs;
    Node* rhs;
    int val;
    int offset;

    Node* cond;
    Node* then;
    Node* els;
};

Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);
Node* new_node_local_variable(int offset);

#endif
