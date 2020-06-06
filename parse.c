// program = (globalVariable | function)*
// function = basetype ident "(" params? ")" "{" stmt* "}"
// basetype = ("int" | "char") "*"*
// params   = param ("," param)*
// param    = basetype ident
// stmt = "return" expr ";"
//       | "if" "(" expr ")" stmt ("else" stmt)?
//       | "while" "(" expr ")" stmt
//       | "for" "(" stmtExpr? ";" expr? ";" stmtExpr? ")" stmt
//       | "{" stmt* "}"
//       | declaration
//       | stmtExpr ";"
// stmtExpr = expr
// expr = assign
// assign = equality ("=" assign)?
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add = mul ("+" mul | "-" mul)*
// mul = unary ("*" unary | "/" unary)*
// unary = ("+" | "-" | "*" | "&" | "sizeof")? unary
//       | postfix
// postfix = primary ("[" expr "]")*
// primary = "(" "{" expressionStatement "}" ")"
//       | "(" expr ")"
//       | ident func-args?
//       | num
//       | string
// func-args = "(" (assign ("," assign)*)? ")"
// declaration = basetype ident ("[" arraySize "]")? ("=" expr)? ";"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "node.h"
#include "parse.h"
#include "tokenize.h"
#include "type.h"

static Token* rp = NULL;

static Variable localVariablesHead = { "head", 0, NULL };
static Variable* localVariablesTail = &localVariablesHead;
static Variable globalVariablesHead = { "head", 0, NULL };
static Variable* globalVariablesTail = &globalVariablesHead;

static Node* expr();
static Type* basetype();
static Node* newAdd(Node* lhs, Node* rhs);
static Node* newSub(Node* lhs, Node* rhs);
static Variable* declarateGlobalVariable(Type* type, char* name);
static Node* statement();

static Token* consume(char* str)
{
    if (rp->kind != TOKEN_RESERVED
        || strlen(rp->str) != strlen(str)
        || strncmp(rp->str, str, strlen(rp->str))) {
        return NULL;
    }
    Token* result = rp;
    rp = rp->next;
    return result;
}

static void expect(char* str)
{
    if (rp->kind != TOKEN_RESERVED
        || strlen(rp->str) != strlen(str)
        || strncmp(rp->str, str, strlen(rp->str))) {
        error("unexpected token");
    }
    rp = rp->next;
}

static int expectNumber()
{
    if (rp->kind != TOKEN_NUMBER)
        error("unexpected non number token");
    int val = rp->val;
    rp = rp->next;
    return val;
}

static char* expectIdentifier()
{
    if (rp->kind != TOKEN_IDENTIFIER) {
        error("invalid token");
    }
    char* name = rp->str;
    rp = rp->next;
    return name;
}

static bool peek(char* str)
{
    return rp->kind == TOKEN_RESERVED
        && strlen(rp->str) == strlen(str)
        && strncmp(rp->str, str, strlen(rp->str)) == 0;
}

static Variable* variable(char* str)
{
    for (Variable* v = localVariablesHead.next; v; v = v->next) {
        if (strncmp(v->name, str, strlen(str)) == 0) {
            return v;
        }
    }
    for (Variable* v = globalVariablesHead.next; v; v = v->next) {
        if (strncmp(v->name, str, strlen(str)) == 0) {
            return v;
        }
    }
    error("undefined variable");
    return NULL;
}

static Token* consumeIdentifier()
{
    if (rp->kind == TOKEN_IDENTIFIER) {
        Token* identifier = rp;
        rp = rp->next;
        return identifier;
    }
    return NULL;
}

static Node* identifier()
{
    Node* node = newNodeVariable(variable(rp->str));
    rp = rp->next;
    return node;
}

static void functionArguments(Node* function)
{
    Node dummy = {};
    Node* tail = &dummy;
    while (!consume(")")) {
        tail->next = expr();
        tail = tail->next;
        consume(",");
    }
    function->args = dummy.next;
}

static char* stringLabel()
{
    static int tag = 0;
    char* label = calloc(20, sizeof(char));
    sprintf(label, "str.literal.%d", tag++);
    return label;
}

static Node* expressionStatement()
{
    Node* node = newNode(NODE_BLOCK, NULL, NULL);
    Node dummy = {};
    Node* tail = &dummy;
    while (!consume("}")) {
        tail->next = statement();
        tail = tail->next;
    }
    node->statements = dummy.next;
    expect(")");
    return node;
}

// primary = "(" "{" expressionStatement "}" ")"
//       | "(" expr ")"
//       | ident func-args?
//       | num
//       | string
static Node* primary()
{
    if (consume("(")) {
        if (consume("{")) {
            return expressionStatement();
        }
        Node* node = expr();
        expect(")");
        return node;
    }

    if (rp->kind == TOKEN_NUMBER) {
        return newNodeNum(expectNumber());
    }

    Token* identifier = consumeIdentifier();
    if (identifier) {
        // Function call
        if (consume("(")) {
            Node* node = newNode(NODE_CALL, NULL, NULL);
            node->name = identifier->str;
            functionArguments(node);
            return node;
        }
        // Variable
        Node* node = newNodeVariable(variable(identifier->str));
        return node;
    }

    if (rp->kind == TOKEN_STRING) {
        char* label = stringLabel();
        Type* type = arrayOf(&CHAR_TYPE, strlen(rp->str));
        declarateGlobalVariable(type, label);
        globalVariablesTail->string = rp->str;
        Node* node = newNodeVariable(variable(label));
        rp = rp->next;
        return node;
    }

    error("invalid token.");
    return NULL;
}

// postfix = primary ("[" expr "]")*
static Node* postfix()
{
    Node* node = primary();
    while (consume("[")) {
        Node* index = expr();
        node = newAdd(node, index);
        node = newNode(NODE_DEREF, node, NULL);
        expect("]");
    }
    return node;
}

// unary = ("+" | "-" | "*" | "&" | "sizeof")? unary
//       | postfix
static Node* unary()
{
    if (consume("+")) {
        return unary();
    } else if (consume("-")) {
        return newNode(NODE_SUBTRACTION, newNodeNum(0), unary());
    } else if (consume("&")) {
        return newNode(NODE_ADDR, unary(), NULL);
    } else if (consume("*")) {
        return newNode(NODE_DEREF, unary(), NULL);
    } else if (consume("sizeof")) {
        Node* node = expr();
        addType(node);
        return newNodeNum(size(node->type));
    }
    return postfix();
}

// mul = unary ("*" unary | "/" unary)*
static Node* mul()
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

static Node* newAdd(Node* lhs, Node* rhs)
{
    addType(lhs);
    addType(rhs);
    TypeKind lType = lhs->type->kind;
    TypeKind rType = rhs->type->kind;
    if ((lType == TYPE_INT || lType == TYPE_CHAR)
        && (rType == TYPE_INT || rType == TYPE_CHAR)) {
        return newNode(NODE_ADDITION, lhs, rhs);
    }
    if ((lType == TYPE_POINTER || lType == TYPE_ARRAY)
        && (rType == TYPE_INT || rType == TYPE_CHAR)) {
        return newNode(NODE_POINTER_ADDITION, lhs, rhs);
    }
    error("invalid addition");
    return NULL;
}

static Node* newSub(Node* lhs, Node* rhs)
{
    addType(lhs);
    addType(rhs);
    TypeKind lType = lhs->type->kind;
    TypeKind rType = rhs->type->kind;
    if ((lType == TYPE_INT || lType == TYPE_CHAR)
        && (rType == TYPE_INT || rType == TYPE_CHAR)) {
        return newNode(NODE_SUBTRACTION, lhs, rhs);
    }
    if ((lType == TYPE_POINTER || lType == TYPE_ARRAY)
        && (rType == TYPE_INT || rType == TYPE_CHAR)) {
        return newNode(NODE_POINTER_SUBTRACTION, lhs, rhs);
    }
    error("invalid subtraction");
    return NULL;
}

// add = mul ("+" mul | "-" mul)*
static Node* add()
{
    Node* node = mul();
    for (;;) {
        if (consume("+")) {
            node = newAdd(node, mul());
        } else if (consume("-")) {
            node = newSub(node, mul());
        } else {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node* relational()
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

// equality = relational ("==" relational | "!=" relational)*
static Node* equality()
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

// assign = equality ("=" assign)?
static Node* assign()
{
    Node* node = equality();
    if (consume("=")) {
        node = newNode(NODE_ASSIGNMENT, node, assign());
    }
    return node;
}

// expr = assign
static Node* expr()
{
    return assign();
}

static Variable* declarateLocalVariable(Type* type, char* name)
{
    for (Variable* v = localVariablesHead.next; v; v = v->next) {
        if (strcmp(v->name, name) == 0) {
            error("duplicated declaration of a local variable named %s", name);
            return NULL;
        }
    }
    Variable* v = calloc(1, sizeof(Variable));
    v->name = name;
    v->type = type;
    v->isGlobal = false;
    v->offset = localVariablesTail->offset + size(type);
    localVariablesTail->next = v;
    localVariablesTail = localVariablesTail->next;
    return v;
}

// declaration = basetype ident ("[" arraySize "]")? ";"
static Node* declaration()
{
    Type* type = basetype();
    char* name = expectIdentifier();
    if (consume("[")) {
        int size = expectNumber();
        type = arrayOf(type, size);
        expect("]");
    }
    Variable* v = declarateLocalVariable(type, name);
    expect(";");
    return newNode(NODE_NULL, NULL, NULL);
}

// stmtExpr = expr
static Node* statementExpression()
{
    return newNode(NODE_STATEMENT_EXPRESSION, expr(), NULL);
}

// stmt = "return" expr ";"
//       | "if" "(" expr ")" stmt ("else" stmt)?
//       | "while" "(" expr ")" stmt
//       | "for" "(" stmtExpr? ";" expr? ";" stmtExpr? ")" stmt
//       | "{" stmt* "}"
//       | declaration
//       | stmtExpr ";"
static Node* statement()
{
    Node* node = NULL;
    if (peek("int") || peek("char")) {
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
            node->init = statementExpression();
            expect(";");
        }
        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = statementExpression();
            expect(")");
        }
        node->body = statement();
    } else if (consume("{")) {
        node = newNode(NODE_BLOCK, NULL, NULL);
        Node dummy = {};
        Node* tail = &dummy;
        while (!consume("}")) {
            tail->next = statement();
            tail = tail->next;
        }
        node->statements = dummy.next;
    } else {
        node = statementExpression();
        expect(";");
    }
    return node;
}

// basetype = ("int" | "char") "*"*
static Type* basetype()
{
    Type* type;
    if (consume("int")) {
        type = &INT_TYPE;
    } else if (consume("char")) {
        type = &CHAR_TYPE;
    } else {
        error("unexpected basetype");
        return NULL;
    }
    while (consume("*")) {
        type = pointerTo(type);
    }
    return type;
}

// param    = basetype ident
static Node* param()
{
    Type* type = basetype();
    char* name = expectIdentifier();
    Variable* localVariable = declarateLocalVariable(type, name);
    Node* node = newNode(NODE_NULL, NULL, NULL);
    node->type = type;
    return node;
}

// params   = param ("," param)*
static Node* params()
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

static int getStackSize()
{
    Variable* tail = &localVariablesHead;
    while (tail->next) {
        tail = tail->next;
    }
    return tail->offset;
}

// function = basetype ident "(" params? ")" "{" stmt* "}"
static Function* function(Type* type, char* name)
{
    localVariablesHead.next = NULL;
    localVariablesTail = &localVariablesHead;
    Function* func = calloc(1, sizeof(Function));
    func->name = name;
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
    func->statements = head.next;
    func->localVariables = localVariablesHead.next;
    func->stackSize = getStackSize();
    addType(func->statements);
    return func;
}

static Variable* declarateGlobalVariable(Type* type, char* name)
{
    for (Variable* v = globalVariablesHead.next; v; v = v->next) {
        if (strcmp(v->name, name) == 0) {
            error("duplicated declaration of a global variable named %s", name);
            return NULL;
        }
    }
    Variable* v = calloc(1, sizeof(Variable));
    v->name = name;
    v->type = type;
    v->isGlobal = true;
    globalVariablesTail->next = v;
    globalVariablesTail = globalVariablesTail->next;
    return v;
}

// program = (globalVariable | function)*
static Program* program()
{
    Function dummyFunction = {};
    Function* functionTail = &dummyFunction;

    while (rp->kind != TOKEN_EOF) {
        Type* type = basetype();
        char* name = expectIdentifier();
        if (consume("(")) {
            functionTail->next = function(type, name);
            functionTail = functionTail->next;
        } else {
            if (consume("[")) {
                int size = expectNumber();
                type = arrayOf(type, size);
                expect("]");
            }
            declarateGlobalVariable(type, name);
            expect(";");
        }
    }

    Program* p = calloc(1, sizeof(Program));
    p->functions = dummyFunction.next;
    p->globalVariables = globalVariablesHead.next;
    return p;
}

Program* parse(Token* tokens)
{
    rp = tokens;
    return program();
}
