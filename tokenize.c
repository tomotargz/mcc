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

Token* new_token(TokenKind kind, Token* cur, char* str, int len)
{
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
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
            current = new_token(TK_RESERVED, current, it, 2);
            it += 2;
        } else if (startsWith(it, "<")
            || startsWith(it, ">")
            || startsWith(it, "<")
            || startsWith(it, "+")
            || startsWith(it, "-")
            || startsWith(it, "*")
            || startsWith(it, "/")
            || startsWith(it, "(")
            || startsWith(it, ")")) {
            current = new_token(TK_RESERVED, current, it, 1);
            ++it;
        } else if (isdigit(*it)) {
            current = new_token(TK_NUM, current, it, 0);
            current->val = strtol(it, &it, 10);
        } else {
            error_at(it, source, "トークナイズできません");
        }
    }

    new_token(TK_EOF, current, it, 0);
    return head.next;
}
