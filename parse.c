// program = (globalVariable | function)*
// globalVariable = basetype identifier ("[" arraySize "]")? ("=" expression)?
// function = basetype identifier "(" parameters? ")" "{" statement* "}"
// basetype = ("int" | "char") "*"*
// parameters = parameter ("," parameter)*
// parameter = basetype identifier
// statement = "return" expression ";"
//           | "if" "(" expression ")" statement ("else" statement)?
//           | "while" "(" expression ")" statement
//           | "for" "(" (localVariable | statementExpression)? ";" expression? ";" statementExpression? ")" statement
//           | "{" statement* "}"
//           | localVariable ";"
//           | statementExpression ";"
// statementExpression = expression
// expression = assign
// assign = equality ("=" assign)?
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add = multiplication ("+" multiplication | "-" multiplication)*
// multiplication = unary ("*" unary | "/" unary)*
// unary = ("+" | "-" | "*" | "&" | "sizeof")? unary
//       | postfix
// postfix = primary ("[" expression "]")*
// primary = "(" "{" expressionStatement "}" ")"
//         | "(" expression ")"
//         | identifier arguments?
//         | number
//         | string
// arguments = "(" (expression ("," expression)*)? ")"
// localVariable = basetype identifier ("[" arraySize "]")? ("=" expression)?
// localVariableInitializer = expression | "{" (expression ("," expression)*)? "}"

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
#include "variable.h"

static char* src;
static char* file;
static Token* rp = NULL;

static VariableList* globalVariables;
static VariableList* localVariables;
static VariableList* scope;

static Node* expression();
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

static char* consumeString()
{
    if (rp->kind == TOKEN_STRING) {
        char* string = rp->str;
        rp = rp->next;
        return string;
    }
    return NULL;
}

static void expect(char* str)
{
    if (rp->kind != TOKEN_RESERVED
        || strlen(rp->str) != strlen(str)
        || strncmp(rp->str, str, strlen(rp->str))) {
        error_at(rp->pos, src, file, "unexpected token");
    }
    rp = rp->next;
}

static int expectNumber()
{
    if (rp->kind != TOKEN_NUMBER)
        error_at(rp->pos, src, file, "unexpected non number token");
    int val = rp->val;
    rp = rp->next;
    return val;
}

static char* expectIdentifier()
{
    if (rp->kind != TOKEN_IDENTIFIER) {
        error_at(rp->pos, src, file, "unexpected non identifier token");
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
    for (VariableList* l = scope; l; l = l->next) {
        if (strlen(str) == strlen(l->variable->name)
            && strncmp(str, l->variable->name, strlen(str)) == 0) {
            return l->variable;
        }
    }
    error_at(rp->pos, src, file, "undefined variable");
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

// arguments = "(" (expression ("," expression)*)? ")"
static void arguments(Node* function)
{
    Node dummy = {};
    Node* tail = &dummy;
    while (!consume(")")) {
        tail->next = expression();
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
    VariableList* currentScope = scope;
    Node* node = newNode(NODE_BLOCK, NULL, NULL);
    Node dummy = {};
    Node* prev = NULL;
    Node* curr = &dummy;
    while (!consume("}")) {
        curr->next = statement();
        prev = curr;
        curr = curr->next;
    }
    if (curr->kind != NODE_STATEMENT_EXPRESSION) {
        error_at(rp->pos, src, file, "expression statement must end with statement expression");
    }
    prev->next = curr->lhs;
    node->statements = dummy.next;
    expect(")");
    scope = currentScope;
    return node;
}

// primary = "(" "{" expressionStatement "}" ")"
//         | "(" expression ")"
//         | identifier arguments?
//         | number
//         | string
static Node* primary()
{
    if (consume("(")) {
        if (consume("{")) {
            Node* node = expressionStatement();
            return node;
        }
        Node* node = expression();
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
            arguments(node);
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
        globalVariables->variable->string = rp->str;
        Node* node = newNodeVariable(variable(label));
        rp = rp->next;
        return node;
    }

    error_at(rp->pos, src, file, "invalid token.");
    return NULL;
}

// postfix = primary ("[" expression "]")*
static Node* postfix()
{
    Node* node = primary();
    while (consume("[")) {
        Node* index = expression();
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
        Node* node = expression();
        addType(node);
        return newNodeNum(size(node->type));
    }
    return postfix();
}

// multiplication = unary ("*" unary | "/" unary)*
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
    error_at(rp->pos, src, file, "invalid addition");
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
    error_at(rp->pos, src, file, "invalid subtraction");
    return NULL;
}

// add = multiplication ("+" multiplication | "-" multiplication)*
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

// expression = assign
static Node* expression()
{
    return assign();
}

static Variable* declarateLocalVariable(Type* type, char* name)
{
    Variable* v = calloc(1, sizeof(Variable));
    v->name = name;
    v->type = type;
    v->isGlobal = false;
    if (localVariables) {
        v->offset = localVariables->variable->offset + size(type);
    } else {
        v->offset = size(type);
    }
    VariableList* l = calloc(1, sizeof(VariableList));
    l->variable = v;
    l->next = localVariables;
    localVariables = l;
    VariableList* ls = calloc(1, sizeof(VariableList));
    ls->variable = v;
    ls->next = scope;
    scope = ls;
    return v;
}

static Node* assginToArray(Variable* array, int index, Node* value)
{
    Node* node = newNodeVariable(array);
    node = newAdd(node, newNodeNum(index));
    node = newNode(NODE_DEREF, node, NULL);
    node = newNode(NODE_ASSIGNMENT, node, value);
    return newNode(NODE_STATEMENT_EXPRESSION, node, NULL);
}

// localVariableInitializer = expression | "{" (expression ("," expression)*)? "}"
static Node* localVariableInitializer(Variable* v)
{
    if (v->type->kind == TYPE_ARRAY) {
        int i = 0;
        Node* block = newNode(NODE_BLOCK, NULL, NULL);
        char* str = consumeString();
        if (str) {
            int len = strlen(str);
            for (; i < len; i++) {
                Node* node = assginToArray(v, i, newNodeNum(str[i]));
                node->next = block->statements;
                block->statements = node;
            }
            Node* node = assginToArray(v, i++, newNodeNum('\0'));
            node->next = block->statements;
            block->statements = node;
        } else {
            expect("{");
            for (; !consume("}"); i++) {
                Node* node = assginToArray(v, i, expression());
                node->next = block->statements;
                block->statements = node;
                consume(",");
            }
        }
        if (v->type->arraySize == 0) {
            v->type->arraySize = i;
            return block;
        }
        for (; i < v->type->arraySize; i++) {
            Node* node = assginToArray(v, i, newNodeNum(0));
            node->next = block->statements;
            block->statements = node;
        }
        return block;
    }
    Node* node = newNodeVariable(v);
    node = newNode(NODE_ASSIGNMENT, node, expression());
    node = newNode(NODE_STATEMENT_EXPRESSION, node, NULL);
    return node;
}

// declaration = basetype ident ("[" arraySize? "]")? ("=" localVariableInitializer)? ";"
static Node* localVariable()
{
    Type* type = basetype();
    char* name = expectIdentifier();
    if (consume("[")) {
        if (consume("]")) {
            type = arrayOf(type, 0);
        } else {
            type = arrayOf(type, expectNumber());
            expect("]");
        }
    }
    Variable* v = declarateLocalVariable(type, name);
    if (consume("=")) {
        return localVariableInitializer(v);
    }
    return newNode(NODE_NULL, NULL, NULL);
}

// statementExpression = expression
static Node* statementExpression()
{
    Node* node = newNode(NODE_STATEMENT_EXPRESSION, expression(), NULL);
    return node;
}

bool isTypeName()
{
    return peek("int") || peek("char");
}

// statement = "return" expression ";"
//           | "if" "(" expression ")" statement ("else" statement)?
//           | "while" "(" expression ")" statement
//           | "for" "(" (localVariable | statementExpression)? ";" expression? ";" statementExpression? ")" statement
//           | "{" statement* "}"
//           | localVariable ";"
//           | statementExpression ";"
static Node* statement()
{
    Node* node = NULL;
    if (isTypeName()) {
        node = localVariable();
        expect(";");
    } else if (consume("return")) {
        node = newNode(NODE_RETURN, expression(), NULL);
        expect(";");
    } else if (consume("if")) {
        node = newNode(NODE_IF, NULL, NULL);
        expect("(");
        node->cond = expression();
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
        node->cond = expression();
        expect(")");
        node->body = statement();
    } else if (consume("for")) {
        node = newNode(NODE_FOR, NULL, NULL);
        expect("(");
        if (!consume(";")) {
            if (isTypeName()) {
                node->init = localVariable();
            } else {
                node->init = statementExpression();
            }
            expect(";");
        }
        if (!consume(";")) {
            node->cond = expression();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = statementExpression();
            expect(")");
        }
        node->body = statement();
    } else if (consume("{")) {
        VariableList* currentScope = scope;
        node = newNode(NODE_BLOCK, NULL, NULL);
        Node dummy = {};
        Node* tail = &dummy;
        while (!consume("}")) {
            tail->next = statement();
            tail = tail->next;
        }
        node->statements = dummy.next;
        scope = currentScope;
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
        error_at(rp->pos, src, file, "unexpected basetype");
        return NULL;
    }
    while (consume("*")) {
        type = pointerTo(type);
    }
    return type;
}

// parameter = basetype identifier
static Node* parameter()
{
    Type* type = basetype();
    char* name = expectIdentifier();
    Variable* localVariable = declarateLocalVariable(type, name);
    Node* node = newNode(NODE_NULL, NULL, NULL);
    node->type = type;
    return node;
}

// parameters = parameter ("," parameter)*
static Node* parameters()
{
    Node head = {};
    Node* tail = &head;
    tail->next = parameter();
    tail = tail->next;
    while (consume(",")) {
        tail->next = parameter();
        tail = tail->next;
    }
    return head.next;
}

// function = basetype identifier "(" parameters? ")" "{" statement* "}"
static Function* function(Type* type, char* name)
{
    localVariables = NULL;
    VariableList* currentScope = scope;
    Function* func = calloc(1, sizeof(Function));
    func->name = name;
    if (!consume(")")) {
        func->params = parameters();
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
    func->localVariables = localVariables;
    if (localVariables) {
        func->stackSize = localVariables->variable->offset;
    } else {
        func->stackSize = 0;
    }
    addType(func->statements);
    scope = currentScope;
    return func;
}

static Variable* declarateGlobalVariable(Type* type, char* name)
{
    Variable* v = calloc(1, sizeof(Variable));
    v->name = name;
    v->type = type;
    v->isGlobal = true;
    VariableList* l = calloc(1, sizeof(VariableList));
    l->variable = v;
    l->next = globalVariables;
    globalVariables = l;
    VariableList* ls = calloc(1, sizeof(VariableList));
    ls->variable = v;
    ls->next = scope;
    scope = ls;
    return v;
}

// globalVariableInitializer = number
static void globalVariableInitializer(Variable* v)
{
    v->initialValue = calloc(1, sizeof(InitialValue));
    char* str = consumeString();
    if (str) {
        if (v->type->kind == TYPE_POINTER
            && v->type->pointerTo->kind == TYPE_CHAR) {
            char* label = stringLabel();
            Type* type = arrayOf(&CHAR_TYPE, strlen(str));
            declarateGlobalVariable(type, label);
            globalVariables->variable->string = str;
            v->initialValue->label = label;
            return;
        } else if (v->type->kind == TYPE_ARRAY
            && v->type->arrayOf->kind == TYPE_CHAR) {
            v->initialValue->string = str;
            if (v->type->arraySize == 0) {
                v->type->arraySize = strlen(str) + 1;
            }
            return;
        }
        error("hoge");
        return;
    }
    if (consume("&")) {
        char* label = expectIdentifier();
        v->initialValue->label = label;
        return;
    }
    if (consume("{")) {
        int i = 0;
        ValueList dummy;
        ValueList* tail = &dummy;
        while (!consume("}")) {
            ValueList* val = calloc(1, sizeof(ValueList));
            val->value = expectNumber();
            tail->next = val;
            tail = val;
            consume(",");
            ++i;
        }
        if (v->type->arraySize == 0) {
            v->type->arraySize = i;
        }
        for (; i < v->type->arraySize; i++) {
            ValueList* val = calloc(1, sizeof(ValueList));
            val->value = 0;
            tail->next = val;
            tail = val;
        }
        v->initialValue->valueList = dummy.next;
        return;
    }
    ValueList* l = calloc(1, sizeof(ValueList));
    l->value = expectNumber();
    v->initialValue->valueList = l;
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
                if (consume("]")) {
                    type = arrayOf(type, 0);
                } else {
                    type = arrayOf(type, expectNumber());
                    expect("]");
                }
            }
            Variable* v = declarateGlobalVariable(type, name);
            if (consume("=")) {
                globalVariableInitializer(v);
            }
            expect(";");
        }
    }

    Program* p = calloc(1, sizeof(Program));
    p->functions = dummyFunction.next;
    p->globalVariables = globalVariables;
    return p;
}

Program* parse(Token* tokens, char* source, char* fileName)
{
    rp = tokens;
    src = source;
    file = fileName;
    return program();
}
