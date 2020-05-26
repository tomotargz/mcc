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
// unary = ("+" | "-" | "*" | "&" | "sizeof")? unary
//       | primary
// primary = "(" expr ")" | ident func-args? | num
// func-args = "(" (assign ("," assign)*)? ")"
// declaration = basetype ident ("[" arraySize "]")? ("=" expr)? ";"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "localVariable.h"
#include "node.h"
#include "parse.h"
#include "tokenize.h"
#include "type.h"

static Token* crr = NULL;

static LocalVariable localVariablesHead = { "head", 0, NULL };
static LocalVariable* localVariablesTail = &localVariablesHead;

Node* expr();
Type* basetype();

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

bool peek(char* str)
{
    return crr->kind == TOKEN_RESERVED
        && strlen(crr->str) == strlen(str)
        && strncmp(crr->str, str, strlen(crr->str)) == 0;
}

LocalVariable* localVariable(char* str)
{
    for (LocalVariable* v = localVariablesHead.next; v; v = v->next) {
        if (strncmp(v->name, str, strlen(str)) == 0) {
            return v;
        }
    }
    error("undefined variable");
    return NULL;
}

Token* consumeIdentifier()
{
    if (crr->kind == TOKEN_IDENTIFIER) {
        Token* identifier = crr;
        crr = crr->next;
        return identifier;
    }
    return NULL;
}

Node* identifier()
{
    Node* node = newNodeLocalVariable(localVariable(crr->str));
    crr = crr->next;
    return node;
}

void functionArguments(Node* function)
{
    int i = 0;
    while (!consume(")")) {
        function->args[i] = expr();
        consume(",");
        ++i;
    }
    function->args[i] = NULL;
}

// // primary = num
// // | "(" expr ")"
// // | identifier
// // | call
// primary = "(" expr ")" | ident func-args? | num
Node* primary()
{
    if (crr->kind == TOKEN_NUMBER) {
        return newNodeNum(expectNumber());
    }

    if (consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
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
        Node* node = newNodeLocalVariable(localVariable(identifier->str));
        return node;
    }

    error("invalid token.");
    return NULL;
}

// unary = ("+" | "-" | "*" | "&" | "sizeof")? unary
//       | primary
Node* unary()
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
        if (node->type->type == TYPE_INT) {
            return newNodeNum(4);
        } else {
            return newNodeNum(8);
        }
    }
    return primary();
}

// mul = unary ("*" unary | "/" unary)*
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

Node* newAdd(Node* lhs, Node* rhs)
{
    addType(lhs);
    addType(rhs);
    if (lhs->type->type == TYPE_INT && rhs->type->type == TYPE_INT) {
        return newNode(NODE_ADDITION, lhs, rhs);
    }
    if (lhs->type->type == TYPE_POINTER && rhs->type->type == TYPE_INT) {
        return newNode(NODE_POINTER_ADDITION, lhs, rhs);
    }
    error("invalid addition");
    return NULL;
}

Node* newSub(Node* lhs, Node* rhs)
{
    addType(lhs);
    addType(rhs);
    if (lhs->type->type == TYPE_INT && rhs->type->type == TYPE_INT) {
        return newNode(NODE_SUBTRACTION, lhs, rhs);
    }
    if (lhs->type->type == TYPE_POINTER && rhs->type->type == TYPE_INT) {
        return newNode(NODE_POINTER_SUBTRACTION, lhs, rhs);
    }
    error("invalid subtraction");
    return NULL;
}

// add = mul ("+" mul | "-" mul)*
Node* add()
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

// equality = relational ("==" relational | "!=" relational)*
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

// assign = equality ("=" assign)?
Node* assign()
{
    Node* node = equality();
    if (consume("=")) {
        node = newNode(NODE_ASSIGNMENT, node, assign());
    }
    return node;
}

// expr = assign
Node* expr()
{
    return assign();
}

LocalVariable* declarateLocalVariable(Type* type)
{
    LocalVariable* prev = &localVariablesHead;
    for (LocalVariable* curr = localVariablesHead.next; curr; curr = curr->next, prev = prev->next) {
        if (strcmp(curr->name, crr->str) == 0) {
            error("duplicated declaration");
        }
    }
    LocalVariable* newLocalVariable = calloc(1, sizeof(LocalVariable));
    newLocalVariable->name = crr->str;
    if (type->type == TYPE_INT) {
        newLocalVariable->offset = prev->offset + 4;
    } else if (type->type == TYPE_POINTER) {
        newLocalVariable->offset = prev->offset + 8;
    } else {
        error("invalid type");
    }
    newLocalVariable->next = NULL;
    newLocalVariable->type = type;
    prev->next = newLocalVariable;
    crr = crr->next;
    return newLocalVariable;
}

// declaration = basetype ident ("[" arraySize "]")? ";"
Node* declaration()
{
    Type* type = basetype();
    LocalVariable* localVariable = declarateLocalVariable(type);
    if (consume("[")) {
        Type* array = calloc(1, sizeof(Type));
        array->type = TYPE_ARRAY;
        array->pointTo = type;
        array->arraySize = expectNumber();
        localVariable->type = array;
        if (type->type == TYPE_INT) {
            localVariable->offset += (array->arraySize - 1) * 4;
        } else if (type->type == TYPE_POINTER) {
            localVariable->offset += (array->arraySize - 1) * 8;
        } else {
            error("invalid type");
        }
        expect("]");
    }
    expect(";");
    return newNode(NODE_NULL, NULL, NULL);
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
    if (peek("int")) {
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
    declarateLocalVariable(type);
    return newNode(NODE_NULL, NULL, NULL);
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

int getStackSize()
{
    LocalVariable* tail = &localVariablesHead;
    while (tail->next) {
        tail = tail->next;
    }
    return tail->offset;
}

// function = basetype ident "(" params? ")" "{" stmt* "}"
Function* function()
{
    localVariablesHead.next = NULL;
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
    func->localVariables = localVariablesHead.next;
    func->stackSize = getStackSize();
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
