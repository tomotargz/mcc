#ifndef TOKENIZE_H
#define TOKENIZE_H

typedef enum {
    TOKEN_EQ = 256,
    TOKEN_NE,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_EOF,
} TokenKind;

typedef struct Token Token;
struct Token {
    int kind;
    char* str;
    int val;
    Token* next;
};

Token* tokenize(char* source);

#endif
