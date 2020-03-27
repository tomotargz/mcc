/*
Generative Rule

program = statement*
statement = expr ";"
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
primary = (identifier | num | "(" expr ")")
*/
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "node.h"
#include "parse.h"
#include "tokenize.h"

static Token* crr = NULL;
static LVar* lvars = NULL;

Node* expr();

bool consumeStr(char* op)
{
    if (crr->kind != TOKEN_RESERVED
        || crr->len != strlen(op)
        || memcmp(crr->str, op, strlen(op)))
        return false;
    crr = crr->next;
    return true;
}

bool consume(TokenKind k)
{
    if (crr->kind != k) {
        return false;
    }
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

int lvarOffset(char* str, int len)
{
    if (!lvars) {
        LVar* lvar = calloc(1, sizeof(LVar));
        lvar->name = str;
        lvar->len = len;
        lvar->offset = 8;
        lvars = lvar;
        return 8;
    }

    LVar* crr = lvars;
    for (;; crr = crr->next) {
        if (crr->len == len && memcmp(crr->name, str, len) == 0) {
            return crr->offset;
        }
        if (!crr->next) {
            break;
        }
    }
    LVar* lvar = calloc(1, sizeof(LVar));
    lvar->name = str;
    lvar->len = len;
    lvar->offset = crr->offset + 8;
    crr->next = lvar;
    crr = lvar;
    return lvar->offset;
}

Node* identifier()
{
    int offset = lvarOffset(crr->str, crr->len);
    Node* node = new_node_local_variable(offset);
    crr = crr->next;
    return node;
}

Node* primary()
{
    if (consumeStr("(")) {
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
    if (consumeStr("+")) {
        return primary();
    } else if (consumeStr("-")) {
        return new_node(NODE_SUBTRACTION, new_node_num(0), primary());
    }
    return primary();
}

Node* mul()
{
    Node* node = unary();
    for (;;) {
        if (consumeStr("*")) {
            node = new_node(NODE_MULTIPLICATION, node, unary());
        } else if (consumeStr("/")) {
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
        if (consumeStr("+")) {
            node = new_node(NODE_ADDITION, node, mul());
        } else if (consumeStr("-")) {
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
        if (consumeStr("<=")) {
            node = new_node(NODE_LESS_OR_EQUAL, node, add());
        } else if (consumeStr("<")) {
            node = new_node(NODE_LESS_THAN, node, add());
        } else if (consumeStr(">=")) {
            node = new_node(NODE_LESS_OR_EQUAL, add(), node);
        } else if (consumeStr(">")) {
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
        if (consumeStr("==")) {
            node = new_node(NODE_EQUAL, node, relational());
        } else if (consumeStr("!=")) {
            node = new_node(NODE_NOT_EQUAL, node, relational());
        } else {
            return node;
        }
    }
}

Node* assign()
{
    Node* node = equality();
    if (consumeStr("=")) {
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
    Node* node = NULL;
    if (consume(TOKEN_RETURN)) {
        node = new_node(NODE_RETURN, expr(), NULL);
        expect(";");
    } else if (consume(TOKEN_IF)) {
        node = calloc(1, sizeof(Node));
        node->kind = NODE_IF;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = statement();
        if (consume(TOKEN_ELSE)) {
            node->els = statement();
        } else {
            node->els = NULL;
        }
    } else if (consume(TOKEN_WHILE)) {
        node = calloc(1, sizeof(Node));
        node->kind = NODE_WHILE;
        expect("(");
        node->cond = expr();
        expect(")");
        node->body = statement();
    } else if (consume(TOKEN_FOR)) {
        node = calloc(1, sizeof(Node));
        node->kind = NODE_FOR;
        expect("(");
        if (!consumeStr(";")) {
            node->init = expr();
            expect(";");
        }
        if (!consumeStr(";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consumeStr(")")) {
            node->inc = expr();
            expect(")");
        }
        node->body = statement();
    } else {
        node = expr();
        expect(";");
    }
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

ParseResult parse(Token* tokens)
{
    crr = tokens;
    Node** ast = program();
    ParseResult result = { ast, lvars };
    return result;
}
