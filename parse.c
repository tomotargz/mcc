// program = function*
// function = basetype ident "(" params? ")" "{" stmt* "}"
// basetype = "int" "*"*
// params   = param ("," param)*
// param    = basetype ident
// stmt = "return" expr ";"
//       | "if" "(" expr ")" stmt ("else" stmt)?
//       | "while" "(" expr ")" stmt
//       | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//       | "{" stmt* "}"
//       | declaration
//       | expr ";"
// expr = assign
// assign = equality ("=" assign)?
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add = mul ("+" mul | "-" mul)*
// mul = unary ("*" unary | "/" unary)*
// unary = ("+" | "-" | "*" | "&")? unary
//       | primary
// primary = "(" expr ")" | ident func-args? | num
// func-args = "(" (assign ("," assign)*)? ")"
// declaration = basetype ident ("=" expr) ";"

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
static LocalVariable* localVariables = NULL;
static int stackSize = 0;

Node* expr();

Token* consume(char* str)
{
    if (crr->kind != TOKEN_RESERVED
        || strlen(crr->str) != strlen(str)
        || strncmp(crr->str, str, strlen(crr->str))) {
        return NULL;
    }
    Token* result = crr;
    crr = crr->next;
    return result;
}

void expect(char* str)
{
    if (crr->kind != TOKEN_RESERVED
        || strlen(crr->str) != strlen(str)
        || strncmp(crr->str, str, strlen(crr->str))) {
        error("unexpected token");
    }
    crr = crr->next;
}

int expectNumber()
{
    if (crr->kind != TOKEN_NUMBER)
        error("unexpected non number token");
    int val = crr->val;
    crr = crr->next;
    return val;
}

int lvarOffset(char* str)
{
    for (LocalVariable* v = localVariables; v; v = v->next) {
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
    Node* node = newNodeLocalVariable(offset);
    crr = crr->next;
    return node;
}

// call = identifier "(" ")" | identifier "(" expr ("," expr)* ")"
Node* call()
{
    Node* node = newNode(NODE_CALL, NULL, NULL);
    node->name = crr->str;
    crr = crr->next;
    expect("(");
    int i = 0;
    while (!consume(")")) {
        node->args[i] = expr();
        consume(",");
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
        return newNodeNum(expectNumber());
    } else if (consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
    } else if (!strncmp(crr->next->str, "(", 1)) {
        return call();
    } else {
        return identifier();
    }
}

Node* unary()
{
    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return newNode(NODE_SUBTRACTION, newNodeNum(0), primary());
    } else if (consume("&")) {
        return newNode(NODE_ADDR, identifier(), NULL);
    } else if (consume("*")) {
        return newNode(NODE_DEREF, identifier(), NULL);
    }
    return primary();
}

Node* mul()
{
    Node* node = unary();
    for (;;) {
        if (consume("*")) {
            node = newNode(NODE_MULTIPLICATION, node, unary());
        } else if (consume("/")) {
            node = newNode(NODE_DIVISION, node, unary());
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
            node = newNode(NODE_ADDITION, node, mul());
        } else if (consume("-")) {
            node = newNode(NODE_SUBTRACTION, node, mul());
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
            node = newNode(NODE_LESS_OR_EQUAL, node, add());
        } else if (consume("<")) {
            node = newNode(NODE_LESS_THAN, node, add());
        } else if (consume(">=")) {
            node = newNode(NODE_LESS_OR_EQUAL, add(), node);
        } else if (consume(">")) {
            node = newNode(NODE_LESS_THAN, add(), node);
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
            node = newNode(NODE_EQUAL, node, relational());
        } else if (consume("!=")) {
            node = newNode(NODE_NOT_EQUAL, node, relational());
        } else {
            return node;
        }
    }
}

Node* assign()
{
    Node* node = equality();
    if (consume("=")) {
        node = newNode(NODE_ASSIGNMENT, node, assign());
    }
    return node;
}

Node* expr()
{
    return assign();
}

Node* localVariableDeclaration()
{
    if (!localVariables) {
        LocalVariable* lvar = calloc(1, sizeof(LocalVariable));
        lvar->name = crr->str;
        lvar->offset = 8;
        lvar->next = NULL;
        localVariables = lvar;
        stackSize = 8;
    } else {
        LocalVariable* v = localVariables;
        for (;; v = v->next) {
            if (strcmp(v->name, crr->str) == 0) {
                crr = crr->next;
                return newNode(NODE_NULL, NULL, NULL);
            }
            if (!v->next) {
                break;
            }
        }
        LocalVariable* lvar = calloc(1, sizeof(LocalVariable));
        lvar->name = crr->str;
        lvar->offset = v->offset + 8;
        lvar->next = NULL;
        v->next = lvar;
        stackSize = lvar->offset;
    }
    crr = crr->next;
    return newNode(NODE_NULL, NULL, NULL);
}

Node* declaration()
{
    Node* node = localVariableDeclaration();
    expect(";");
    return node;
}

// stmt = "return" expr ";"
//       | "if" "(" expr ")" stmt ("else" stmt)?
//       | "while" "(" expr ")" stmt
//       | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//       | "{" stmt* "}"
//       | declaration
//       | expr ";"
Node* statement()
{
    Node* node = NULL;
    if (consume("int")) {
        node = declaration();
    } else if (consume("return")) {
        node = newNode(NODE_RETURN, expr(), NULL);
        expect(";");
    } else if (consume("if")) {
        node = newNode(NODE_IF, NULL, NULL);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = statement();
        if (consume("else")) {
            node->els = statement();
        } else {
            node->els = NULL;
        }
    } else if (consume("while")) {
        node = newNode(NODE_WHILE, NULL, NULL);
        expect("(");
        node->cond = expr();
        expect(")");
        node->body = statement();
    } else if (consume("for")) {
        node = newNode(NODE_FOR, NULL, NULL);
        expect("(");
        if (!consume(";")) {
            node->init = expr();
            expect(";");
        }
        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = expr();
            expect(")");
        }
        node->body = statement();
    } else if (consume("{")) {
        node = newNode(NODE_BLOCK, NULL, NULL);
        int i = 0;
        for (; i < 100 && !consume("}"); ++i) {
            node->statements[i] = statement();
        }
        node->statements[i] = NULL;
    } else {
        node = expr();
        expect(";");
    }
    return node;
}

Type INT_TYPE = { TYPE_INT, NULL };

// basetype = "int" "*"*
Type* basetype()
{
    expect("int");

    Type* type = &INT_TYPE;
    while (consume("*")) {
        type = pointerTo(type);
    }
    return type;
}

char* expectIdentifier()
{
    if (crr->kind != TOKEN_IDENTIFIER) {
        error("invalid token");
    }
    char* name = crr->str;
    crr = crr->next;
    return name;
}

// param    = basetype ident
Node* param()
{
    Type* type = basetype();
    return localVariableDeclaration();
}

// params   = param ("," param)*
Node* params()
{
    Node head = {};
    Node* tail = &head;
    tail->next = param();
    tail = tail->next;
    while (consume(",")) {
        tail->next = param();
        tail = tail->next;
    }
    return head.next;
}

// function = basetype ident "(" params? ")" "{" stmt* "}"
Function* function()
{
    localVariables = NULL;
    stackSize = 0;
    Function* func = calloc(1, sizeof(Function));
    basetype();
    if (crr->kind != TOKEN_IDENTIFIER) {
        error("invalid token");
    }
    func->name = expectIdentifier();
    expect("(");
    if (!consume(")")) {
        func->params = params();
        expect(")");
    }
    expect("{");
    Node head;
    head.next = NULL;
    Node* tail = &head;
    while (!consume("}")) {
        tail->next = statement();
        tail = tail->next;
    }
    func->node = head.next;
    func->localVariables = localVariables;
    func->stackSize = stackSize;
    return func;
}

// program = function*
Function* program()
{
    Function head = {};
    Function* tail = &head;
    while (crr->kind != TOKEN_EOF) {
        tail->next = function();
        tail = tail->next;
    }
    return head.next;
}

Function* parse(Token* tokens)
{
    crr = tokens;
    return program();
}
