#ifndef TOKENIZE_H
#define TOKENIZE_H

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

#endif
