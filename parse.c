// Generative Rule
//
// expr = equality
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | "=>" add)*
// add = mul ("+" mul | "-" mul)*
// mul = unary ("*" unary | "/" unary)*
// unary = ("+" | "-")? primary
// primary = (num | "(" expr ")")
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "parse.h"

static Token* current;

Node* expr();

bool consume(char* op)
{
    if (current->kind != TK_RESERVED
        || current->len != strlen(op)
        || memcmp(current->str, op, strlen(op)))
        return false;
    current = current->next;
    return true;
}

void expect(char* op)
{
    if (current->kind != TK_RESERVED
        || current->len != strlen(op)
        || memcmp(current->str, op, strlen(op)))
        // error_at(current->str, "'%s'ではありません", op);
        error("'%s'ではありません", op);
    current = current->next;
}

int expect_number()
{
    if (current->kind != TK_NUM)
        // error_at(current->str, "数ではありません");
        error("数ではありません");
    int val = current->val;
    current = current->next;
    return val;
}

Node* primary()
{
    if (consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
    }
    return new_node_num(expect_number());
}

Node* unary()
{
    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}

Node* mul()
{
    Node* node = unary();
    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
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
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
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
            node = new_node(ND_LE, node, add());
        } else if (consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume(">=")) {
            node = new_node(ND_LE, add(), node);
        } else if (consume(">")) {
            node = new_node(ND_LT, add(), node);
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
            node = new_node(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

Node* expr()
{
    return equality();
}

Node* parse(Token* tokens)
{
    current = tokens;
    return expr();
}
