// program = (globalVariable | function)*
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
//       | postfix
// postfix = primary ("[" expr "]")*
// primary = "(" expr ")" | ident func-args? | num
// func-args = "(" (assign ("," assign)*)? ")"
// declaration = basetype ident ("[" arraySize "]")? ("=" expr)? ";"

#include <ctype.h>
#include <stdbool.h>
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

Node* expr();
Type* basetype();
Node* newAdd(Node* lhs, Node* rhs);
Node* newSub(Node* lhs, Node* rhs);

Token* consume(char* str)
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

void expect(char* str)
{
    if (rp->kind != TOKEN_RESERVED
        || strlen(rp->str) != strlen(str)
        || strncmp(rp->str, str, strlen(rp->str))) {
        error("unexpected token");
    }
    rp = rp->next;
}

int expectNumber()
{
    if (rp->kind != TOKEN_NUMBER)
        error("unexpected non number token");
    int val = rp->val;
    rp = rp->next;
    return val;
}

bool peek(char* str)
{
    return rp->kind == TOKEN_RESERVED
        && strlen(rp->str) == strlen(str)
        && strncmp(rp->str, str, strlen(rp->str)) == 0;
}

Variable* variable(char* str)
{
    for (Variable* v = localVariablesHead.next; v; v = v->next) {
        if (strncmp(v->name, str, strlen(str)) == 0) {
            return v;
        }
    }
    error("undefined variable");
    return NULL;
}

Token* consumeIdentifier()
{
    if (rp->kind == TOKEN_IDENTIFIER) {
        Token* identifier = rp;
        rp = rp->next;
        return identifier;
    }
    return NULL;
}

Node* identifier()
{
    Node* node = newNodeVariable(variable(rp->str));
    rp = rp->next;
    return node;
}

void functionArguments(Node* function)
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

// // primary = num
// // | "(" expr ")"
// // | identifier
// // | call
// primary = "(" expr ")" | ident func-args? | num
Node* primary()
{
    if (rp->kind == TOKEN_NUMBER) {
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
        Node* node = newNodeVariable(variable(identifier->str));
        return node;
    }

    error("invalid token.");
    return NULL;
}

// postfix = primary ("[" expr "]")*
Node* postfix()
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
        if (node->type->kind == TYPE_INT) {
            return newNodeNum(8);
        } else {
            return newNodeNum(8);
        }
    }
    return postfix();
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
    if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT) {
        return newNode(NODE_ADDITION, lhs, rhs);
    }
    if ((lhs->type->kind == TYPE_POINTER || lhs->type->kind == TYPE_ARRAY)
        && rhs->type->kind == TYPE_INT) {
        return newNode(NODE_POINTER_ADDITION, lhs, rhs);
    }
    error("invalid addition");
    return NULL;
}

Node* newSub(Node* lhs, Node* rhs)
{
    addType(lhs);
    addType(rhs);
    if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT) {
        return newNode(NODE_SUBTRACTION, lhs, rhs);
    }
    if ((lhs->type->kind == TYPE_POINTER || lhs->type->kind == TYPE_ARRAY)
        && rhs->type->kind == TYPE_INT) {
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

Variable* declarateLocalVariable(Type* type)
{
    Variable* prev = &localVariablesHead;
    for (Variable* curr = localVariablesHead.next; curr; curr = curr->next, prev = prev->next) {
        if (strcmp(curr->name, rp->str) == 0) {
            error("duplicated declaration");
        }
    }
    Variable* newLocalVariable = calloc(1, sizeof(Variable));
    newLocalVariable->name = rp->str;
    if (type->kind == TYPE_INT) {
        newLocalVariable->offset = prev->offset + 8;
    } else if (type->kind == TYPE_POINTER) {
        newLocalVariable->offset = prev->offset + 8;
    } else {
        error("invalid type");
    }
    newLocalVariable->next = NULL;
    newLocalVariable->type = type;
    prev->next = newLocalVariable;
    rp = rp->next;
    return newLocalVariable;
}

// declaration = basetype ident ("[" arraySize "]")? ";"
Node* declaration()
{
    Type* type = basetype();
    Variable* localVariable = declarateLocalVariable(type);
    if (consume("[")) {
        Type* array = calloc(1, sizeof(Type));
        array->kind = TYPE_ARRAY;
        array->arrayOf = type;
        array->arraySize = expectNumber();
        localVariable->type = array;
        if (type->kind == TYPE_INT) {
            localVariable->offset += (array->arraySize - 1) * 8;
        } else if (type->kind == TYPE_POINTER) {
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
        Node dummy = {};
        Node* tail = &dummy;
        while (!consume("}")) {
            tail->next = statement();
            tail = tail->next;
        }
        node->statements = dummy.next;
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
        type = newPointer(type);
    }
    return type;
}

char* expectIdentifier()
{
    if (rp->kind != TOKEN_IDENTIFIER) {
        error("invalid token");
    }
    char* name = rp->str;
    rp = rp->next;
    return name;
}

// param    = basetype ident
Node* param()
{
    Type* type = basetype();
    Variable* localVariable = declarateLocalVariable(type);
    Node* node = newNode(NODE_NULL, NULL, NULL);
    node->type = type;
    return node;
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
    Variable* tail = &localVariablesHead;
    while (tail->next) {
        tail = tail->next;
    }
    return tail->offset;
}

// function = basetype ident "(" params? ")" "{" stmt* "}"
Function* function(Type* type, char* name)
{
    localVariablesHead.next = NULL;
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
    return func;
}

Variable* globalVariable()
{
    expect(";");
    return calloc(1, sizeof(Variable));
}

// program = (globalVariable | function)*
Program* program()
{
    Function dummyFunction = {};
    Function* functionTail = &dummyFunction;
    Variable dummyGlobalVariable = {};
    Variable* globalVariableTail = &dummyGlobalVariable;

    while (rp->kind != TOKEN_EOF) {
        Type* type = basetype();
        char* name = expectIdentifier();
        if (consume("(")) {
            functionTail->next = function(type, name);
            functionTail = functionTail->next;
        } else {
            globalVariableTail->next = globalVariable();
            globalVariableTail = globalVariableTail->next;
        }
    }

    Program* p = calloc(1, sizeof(Program));
    p->functions = dummyFunction.next;
    p->globalVariables = dummyGlobalVariable.next;
    return p;
}

Program* parse(Token* tokens)
{
    rp = tokens;
    return program();
}
