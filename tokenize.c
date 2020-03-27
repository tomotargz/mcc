#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "tokenize.h"

bool startsWith(char* a, char* b)
{
    return memcmp(a, b, strlen(b)) == 0;
}

Token* new_token(TokenKind kind, Token* cur, char* str, int len, int val)
{
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    tok->val = val;
    cur->next = tok;
    return tok;
}

bool isAlphaDigitUnderbar(char c)
{
    return isalpha(c) || isdigit(c) || c == '_';
}

Token* tokenize(char* source)
{
    Token head;
    head.next = NULL;
    Token* current = &head;

    char* it = source;

    while (*it) {
        if (isspace(*it)) {
            it++;
        } else if (startsWith(it, "==")
            || startsWith(it, "!=")
            || startsWith(it, "<=")
            || startsWith(it, ">=")) {
            current = new_token(TOKEN_RESERVED, current, it, 2, 0);
            it += 2;
        } else if (startsWith(it, "<")
            || startsWith(it, ">")
            || startsWith(it, "<")
            || startsWith(it, "+")
            || startsWith(it, "-")
            || startsWith(it, "*")
            || startsWith(it, "/")
            || startsWith(it, "(")
            || startsWith(it, ")")
            || startsWith(it, ";")
            || startsWith(it, "=")) {
            current = new_token(TOKEN_RESERVED, current, it, 1, 0);
            ++it;
        } else if (isdigit(*it)) {
            current = new_token(TOKEN_NUMBER, current, it, 0, strtol(it, &it, 10));
        } else if (memcmp(it, "return", 6) == 0 && !isAlphaDigitUnderbar(it[6])) {
            current = new_token(TOKEN_RETURN, current, it, 6, 0);
            it += 6;
        } else if (memcmp(it, "if", 2) == 0 && !isAlphaDigitUnderbar(it[2])) {
            current = new_token(TOKEN_IF, current, it, 2, 0);
            it += 2;
        } else if (memcmp(it, "else", 4) == 0 && !isAlphaDigitUnderbar(it[4])) {
            current = new_token(TOKEN_ELSE, current, it, 4, 0);
            it += 4;
        } else if (memcmp(it, "while", 5) == 0 && !isAlphaDigitUnderbar(it[5])) {
            current = new_token(TOKEN_WHILE, current, it, 5, 0);
            it += 5;
        } else if (isalpha(*it) || *it == '_') {
            char* end = it + 1;
            int len = 1;
            while (isAlphaDigitUnderbar(*end)) {
                ++len;
                ++end;
            }
            current = new_token(TOKEN_IDENTIFIER, current, it, len, 0);
            it = end;
        } else {
            error_at(it, source, "トークナイズできません");
        }
    }

    new_token(TOKEN_EOF, current, it, 0, 0);
    return head.next;
}
