// Generative Rule
//
// program = statement*
// statement = expr ";"
// expr = assign
// assign = equality ("=" assign)?
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | "=>" add)*
// add = mul ("+" mul | "-" mul)*
// mul = unary ("*" unary | "/" unary)*
// unary = ("+" | "-")? primary
// primary = (identifier | num | "(" expr ")")
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "node.h"
#include "parse.h"

static Token* crr;

Node* expr();

bool consume(char* op)
{
    if (crr->kind != TOKEN_RESERVED
        || crr->len != strlen(op)
        || memcmp(crr->str, op, strlen(op)))
        return false;
    crr = crr->next;
    return true;
}

void expect(char* op)
{
    if (crr->kind != TOKEN_RESERVED
        || crr->len != strlen(op)
        || memcmp(crr->str, op, strlen(op)))
        // error_at(crr->str, "'%s'ではありません", op);
        error("'%s'ではありません", op);
    crr = crr->next;
}

int expect_number()
{
    if (crr->kind != TOKEN_NUMBER)
        // error_at(crr->str, "数ではありません");
        error("数ではありません");
    int val = crr->val;
    crr = crr->next;
    return val;
}

Node* identifier()
{
    Node* node = new_node_local_variable((crr->str)[0]);
    crr = crr->next;
    return node;
}

Node* primary()
{
    if (consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
    }
    if ('a' <= (crr->str)[0] && (crr->str)[0] <= 'z') {
        return identifier();
    }
    return new_node_num(expect_number());
}

Node* unary()
{
    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(NODE_SUBTRACTION, new_node_num(0), primary());
    }
    return primary();
}

Node* mul()
{
    Node* node = unary();
    for (;;) {
        if (consume("*")) {
            node = new_node(NODE_MULTIPLICATION, node, unary());
        } else if (consume("/")) {
            node = new_node(NODE_DIVISION, node, unary());
        } else {
            return node;
        }
    }
}

Node* add()
{
    Node* node = mul();
    for (;;) {
        if (consume("+")) {
            node = new_node(NODE_ADDITION, node, mul());
        } else if (consume("-")) {
            node = new_node(NODE_SUBTRACTION, node, mul());
        } else {
            return node;
        }
    }
}

Node* relational()
{
    Node* node = add();
    for (;;) {
        if (consume("<=")) {
            node = new_node(NODE_LESS_OR_EQUAL, node, add());
        } else if (consume("<")) {
            node = new_node(NODE_LESS_THAN, node, add());
        } else if (consume(">=")) {
            node = new_node(NODE_LESS_OR_EQUAL, add(), node);
        } else if (consume(">")) {
            node = new_node(NODE_LESS_THAN, add(), node);
        } else {
            return node;
        }
    }
}

Node* equality()
{
    Node* node = relational();
    for (;;) {
        if (consume("==")) {
            node = new_node(NODE_EQUAL, node, relational());
        } else if (consume("!=")) {
            node = new_node(NODE_NOT_EQUAL, node, relational());
        } else {
            return node;
        }
    }
}

Node* assign()
{
    Node* node = equality();
    if (consume("=")) {
        node = new_node(NODE_ASSIGNMENT, node, assign());
    }
    return node;
}

Node* expr()
{
    return assign();
}

Node* statement()
{
    Node* node = expr();
    expect(";");
    return node;
}

Node** program()
{
    static Node* statements[100];
    int i = 0;
    while (crr->kind != TOKEN_EOF) {
        statements[i++] = statement();
    }
    statements[i] = NULL;
    return statements;
}

Node** parse(Token* tokens)
{
    crr = tokens;
    return program();
}
