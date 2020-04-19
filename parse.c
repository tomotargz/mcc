/*
Generative Rule

program = function*

function = "int" identifier "(" ("int" lvarDecl)* ")" "{" statement* "}"

statement = expr ";"
| "int" lvarDecl ";"
| "{" statement* "}"
| "if" "(" expr ")" statement ("else" statement)?
| "while" "(" expr ")" statement
| "for" "(" expr? ";" expr? ";" expr? ")" statement
| return expr ";"

expr = assign

assign = equality ("=" assign)?

equality = relational ("==" relational | "!=" relational)*

relational = add ("<" add | "<=" add | ">" add | "=>" add)*

add = mul ("+" mul | "-" mul)*

mul = unary ("*" unary | "/" unary)*

unary = ("+" | "-")? primary
| "*" identifier
| "&" identifier

primary = num
| "(" expr ")"
| identifier
| identifier "(" ")"
| identifier "(" expr ("," expr)* ")"
*/
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "node.h"
#include "parse.h"
#include "tokenize.h"

static Token* crr = NULL;
static LVar* lvars = NULL;
static int stackSize = 0;

Node* expr();

bool consume(int kind)
{
    if (crr->kind != kind) {
        return false;
    }
    crr = crr->next;
    return true;
}

void expect(int kind)
{
    if (crr->kind != kind) {
        error("invalid token.");
    }
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

int lvarOffset(char* str)
{
    for (LVar* v = lvars; v; v = v->next) {
        if (strcmp(v->name, str) == 0) {
            return v->offset;
        }
    }
    error("undefined variable");
    return 0;
}

Node* identifier()
{
    int offset = lvarOffset(crr->str);
    Node* node = new_node_local_variable(offset);
    crr = crr->next;
    return node;
}

// call = identifier "(" ")" | identifier "(" expr ("," expr)* ")"
Node* call()
{
    Node* node = calloc(1, sizeof(Node));
    node->kind = NODE_CALL;
    node->name = crr->str;
    crr = crr->next;
    expect('(');
    int i = 0;
    while (!consume(')')) {
        node->args[i] = expr();
        consume(',');
        ++i;
    }
    node->args[i] = NULL;
    return node;
}

// primary = num
// | "(" expr ")"
// | identifier
// | call
Node* primary()
{
    if (crr->kind == TOKEN_NUMBER) {
        return new_node_num(expect_number());
    } else if (consume('(')) {
        Node* node = expr();
        expect(')');
        return node;
    } else if (crr->next->kind == '(') {
        return call();
    } else {
        return identifier();
    }
}

Node* unary()
{
    if (consume('+')) {
        return primary();
    } else if (consume('-')) {
        return new_node(NODE_SUBTRACTION, new_node_num(0), primary());
    } else if (consume('&')) {
        return new_node(NODE_ADDR, identifier(), NULL);
    } else if (consume('*')) {
        return new_node(NODE_DEREF, identifier(), NULL);
    }
    return primary();
}

Node* mul()
{
    Node* node = unary();
    for (;;) {
        if (consume('*')) {
            node = new_node(NODE_MULTIPLICATION, node, unary());
        } else if (consume('/')) {
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
        if (consume('+')) {
            node = new_node(NODE_ADDITION, node, mul());
        } else if (consume('-')) {
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
        if (consume(TOKEN_LE)) {
            node = new_node(NODE_LESS_OR_EQUAL, node, add());
        } else if (consume('<')) {
            node = new_node(NODE_LESS_THAN, node, add());
        } else if (consume(TOKEN_GE)) {
            node = new_node(NODE_LESS_OR_EQUAL, add(), node);
        } else if (consume('>')) {
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
        if (consume(TOKEN_EQ)) {
            node = new_node(NODE_EQUAL, node, relational());
        } else if (consume(TOKEN_NE)) {
            node = new_node(NODE_NOT_EQUAL, node, relational());
        } else {
            return node;
        }
    }
}

Node* assign()
{
    Node* node = equality();
    if (consume('=')) {
        node = new_node(NODE_ASSIGNMENT, node, assign());
    }
    return node;
}

Node* expr()
{
    return assign();
}

Node* lvarDecl()
{
    if (!lvars) {
        LVar* lvar = calloc(1, sizeof(LVar));
        lvar->name = crr->str;
        lvar->offset = 8;
        lvar->next = NULL;
        lvars = lvar;
        stackSize = 8;
    } else {
        LVar* v = lvars;
        for (;; v = v->next) {
            if (strcmp(v->name, crr->str) == 0) {
                crr = crr->next;
                return new_node(NODE_NULL, NULL, NULL);
            }
            if (!v->next) {
                break;
            }
        }
        LVar* lvar = calloc(1, sizeof(LVar));
        lvar->name = crr->str;
        lvar->offset = v->offset + 8;
        lvar->next = NULL;
        v->next = lvar;
        stackSize = lvar->offset;
    }
    crr = crr->next;
    return new_node(NODE_NULL, NULL, NULL);
}

Node* statement()
{
    Node* node = NULL;
    if (consume(TOKEN_INT)) {
        node = lvarDecl();
        expect(';');
    } else if (consume(TOKEN_RETURN)) {
        node = new_node(NODE_RETURN, expr(), NULL);
        expect(';');
    } else if (consume(TOKEN_IF)) {
        node = calloc(1, sizeof(Node));
        node->kind = NODE_IF;
        expect('(');
        node->cond = expr();
        expect(')');
        node->then = statement();
        if (consume(TOKEN_ELSE)) {
            node->els = statement();
        } else {
            node->els = NULL;
        }
    } else if (consume(TOKEN_WHILE)) {
        node = calloc(1, sizeof(Node));
        node->kind = NODE_WHILE;
        expect('(');
        node->cond = expr();
        expect(')');
        node->body = statement();
    } else if (consume(TOKEN_FOR)) {
        node = calloc(1, sizeof(Node));
        node->kind = NODE_FOR;
        expect('(');
        if (!consume(';')) {
            node->init = expr();
            expect(';');
        }
        if (!consume(';')) {
            node->cond = expr();
            expect(';');
        }
        if (!consume(')')) {
            node->inc = expr();
            expect(')');
        }
        node->body = statement();
    } else if (consume('{')) {
        node = calloc(1, sizeof(Node));
        node->kind = NODE_BLOCK;
        int i = 0;
        for (; i < 100 && !consume('}'); ++i) {
            node->statements[i] = statement();
        }
        node->statements[i] = NULL;
    } else {
        node = expr();
        expect(';');
    }
    return node;
}

Function* function()
{
    expect(TOKEN_INT);
    if (crr->kind != TOKEN_IDENTIFIER) {
        error("invalid token");
    }
    char* name = crr->str;
    Node head = {};
    Node* pos = &head;
    crr = crr->next;

    Function* func = calloc(1, sizeof(Function));

    lvars = NULL;
    expect('(');
    Node dummyParam = {};
    Node* tail = &dummyParam;
    while (!consume(')')) {
        expect(TOKEN_INT);
        tail->next = lvarDecl();
        tail = tail->next;
        consume(',');
    }
    func->params = dummyParam.next;

    tail->next = NULL;
    expect('{');
    stackSize = 0;
    while (!consume('}')) {
        pos->next = statement();
        pos = pos->next;
    }
    func->name = name;
    func->node = head.next;
    func->lVars = lvars;
    func->stackSize = stackSize;
    return func;
}

Function* program()
{
    Function head = {};
    Function* pos = &head;
    while (crr->kind != TOKEN_EOF) {
        pos->next = function();
        pos = pos->next;
    }
    return head.next;
}

Function* parse(Token* tokens)
{
    crr = tokens;
    return program();
}
