#ifndef TOKENIZE_H
#define TOKENIZE_H

typedef enum {
    TOKEN_RESERVED,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_EOF,
} TokenKind;

typedef struct Token Token;
struct Token {
    TokenKind kind;
    char* str;
    int len;
    int val;
    Token* next;
};

Token* tokenize(char* source);

#endif
