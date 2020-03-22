#ifndef TOKENIZE_H
#define TOKENIZE_H

typedef enum {
    TOKEN_RESERVED,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_EOF,
} TokenKind;

typedef struct Token Token;
struct Token {
    TokenKind kind;
    Token* next;
    int val;
    char* str;
    int len;
};

Token* tokenize(char* source);

#endif
