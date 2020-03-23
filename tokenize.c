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
        } else if (isalpha(*it) || *it == '_') {
            char* end = it + 1;
            int len = 1;
            while (isalpha(*end) || *end == '_' || isdigit(*end)) {
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
