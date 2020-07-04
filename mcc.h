#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// type.c-----------------------------------------------------------------------

typedef enum TypeKind {
    TYPE_VOID,
    TYPE_CHAR,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_LONG,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_ENUM,
    TYPE_NO
} TypeKind;

typedef struct Type Type;

typedef struct Member {
    Type* type;
    char* name;
    int offset;
    struct Member* next;
} Member;

typedef struct Type {
    TypeKind kind;
    int align;
    // pointer
    struct Type* pointerTo;
    // array
    struct Type* arrayOf;
    int arraySize;
    // struct
    Member* members;
    // function
    Type* retType;
} Type;

Type* voidType();
Type* charType();
Type* shortType();
Type* intType();
Type* longType();
Type* enumType();
Type* noType();
Type* pointerTo(Type* to);
Type* arrayOf(Type* type, int size);
int size(Type* type);
int alignOffset(int offset, int align);

// variable.h-------------------------------------------------------------------

typedef struct ValueList {
    struct ValueList* next;
    int value;
} ValueList;

typedef struct InitialValue {
    ValueList* valueList;
    char* label;
    char* string;
} InitialValue;

typedef struct Variable Variable;
struct Variable {
    char* name;
    int offset;
    Type* type;
    bool isGlobal;
    char* string;
    InitialValue* initialValue;
};

typedef struct VariableList {
    struct VariableList* next;
    Variable* variable;
} VariableList;

// node.c-----------------------------------------------------------------------

typedef enum {
    NODE_ADDITION,
    NODE_SUBTRACTION,
    NODE_POINTER_ADDITION,
    NODE_POINTER_SUBTRACTION,
    NODE_MULTIPLICATION,
    NODE_DIVISION,
    NODE_EQUAL,
    NODE_NOT_EQUAL,
    NODE_LESS_THAN,
    NODE_LESS_OR_EQUAL,
    NODE_NUMBER,
    NODE_ASSIGNMENT,
    NODE_LOCAL_VARIABLE,
    NODE_GLOBAL_VARIABLE,
    NODE_CONDITION,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_BLOCK,
    NODE_RETURN,
    NODE_CALL,
    NODE_ADDR,
    NODE_DEREF,
    NODE_STATEMENT_EXPRESSION,
    NODE_MEMBER,
    NODE_PRE_INCREMENT,
    NODE_POST_INCREMENT,
    NODE_PRE_DECREMENT,
    NODE_POST_DECREMENT,
    NODE_NOT,
    NODE_AND,
    NODE_OR,
    NODE_NULL,
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node* next;
    Node* lhs;
    Node* rhs;
    int val;
    Type* type;
    int offset;

    // if(cond)then;else els;
    // while(cond)body;
    // for(init;cond;inc)body;
    Node* cond;
    Node* then;
    Node* els;
    Node* body;
    Node* init;
    Node* inc;

    Node* statements;

    // function
    char* name;
    Node* args;

    // struct member
    Member* member;
};

Node* newNode(NodeKind kind, Node* lhs, Node* rhs);
Node* newNodeNum(int val);
Node* newNodeVariable(Variable* variable);
void addType(Node* tree);

// tokenize.c-------------------------------------------------------------------

typedef enum {
    TOKEN_RESERVED,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_EOF
} TokenKind;

typedef struct Token Token;
struct Token {
    TokenKind kind;
    int val;
    char* str;
    char* pos;
    Token* next;
};

Token* tokenize(char* source, char* fileName);

// parse.c----------------------------------------------------------------------

typedef struct Function Function;
struct Function {
    Function* next;
    char* name;
    Node* params;
    Node* statements;
    VariableList* localVariables;
    int stackSize;
    bool isStatic;
    bool isVariadic;
};

typedef struct {
    VariableList* globalVariables;
    Function* functions;
} Program;

Program* parse(Token* tokens, char* source, char* fileName);

// codegen.c--------------------------------------------------------------------

void generateCode(Program* program);

// error.c----------------------------------------------------------------------

void error(char* format, ...);
void error_at(char* position, char* source, char* fileName, char* format, ...);
void info(char* format, ...);
