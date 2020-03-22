// Generative Rule
//
// program = statement*
// statement = expr ";"
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
    if (current->kind != TOKEN_RESERVED
        || current->len != strlen(op)
        || memcmp(current->str, op, strlen(op)))
        return false;
    current = current->next;
    return true;
}

void expect(char* op)
{
    if (current->kind != TOKEN_RESERVED
        || current->len != strlen(op)
        || memcmp(current->str, op, strlen(op)))
        // error_at(current->str, "'%s'ではありません", op);
        error("'%s'ではありません", op);
    current = current->next;
}

int expect_number()
{
    if (current->kind != TOKEN_NUMBER)
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

Node* expr()
{
    return equality();
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
    while (current->kind != TOKEN_EOF) {
        statements[i++] = statement();
        info("statement %d", i);
    }
    statements[i] = NULL;
    return statements;
}

Node** parse(Token* tokens)
{
    current = tokens;
    return program();
}
