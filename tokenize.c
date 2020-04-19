#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "tokenize.h"

static Token head;
static Token* tail = &head;

bool startsWith(char* a, char* b)
{
    return memcmp(a, b, strlen(b)) == 0;
}

bool isAlphaDigitUnderbar(char c)
{
    return isalpha(c) || isdigit(c) || c == '_';
}

void append(int kind)
{
    Token* t = calloc(1, sizeof(Token));
    t->kind = kind;
    tail->next = t;
    tail = t;
}

Token* tokenize(char* source)
{
    char* pos = source;

    while (*pos) {
        if (isspace(*pos)) {
            pos++;
        } else if (startsWith(pos, "==")) {
            append(TOKEN_EQ);
            pos += 2;
        } else if (startsWith(pos, "!=")) {
            append(TOKEN_NE);
            pos += 2;
        } else if (startsWith(pos, "<=")) {
            append(TOKEN_LE);
            pos += 2;
        } else if (startsWith(pos, ">=")) {
            append(TOKEN_GE);
            pos += 2;
        } else if (startsWith(pos, "<")
            || startsWith(pos, ">")
            || startsWith(pos, "<")
            || startsWith(pos, "+")
            || startsWith(pos, "-")
            || startsWith(pos, "*")
            || startsWith(pos, "/")
            || startsWith(pos, "(")
            || startsWith(pos, ")")
            || startsWith(pos, "{")
            || startsWith(pos, "}")
            || startsWith(pos, ";")
            || startsWith(pos, "=")
            || startsWith(pos, ",")
            || startsWith(pos, "&")) {
            append(*pos);
            ++pos;
        } else if (isdigit(*pos)) {
            append(TOKEN_NUMBER);
            tail->val = strtol(pos, &pos, 10);
        } else if (startsWith(pos, "return") && !isAlphaDigitUnderbar(pos[6])) {
            append(TOKEN_RETURN);
            pos += 6;
        } else if (startsWith(pos, "if") && !isAlphaDigitUnderbar(pos[2])) {
            append(TOKEN_IF);
            pos += 2;
        } else if (startsWith(pos, "else") && !isAlphaDigitUnderbar(pos[4])) {
            append(TOKEN_ELSE);
            pos += 4;
        } else if (startsWith(pos, "while") && !isAlphaDigitUnderbar(pos[5])) {
            append(TOKEN_WHILE);
            pos += 5;
        } else if (startsWith(pos, "for") && !isAlphaDigitUnderbar(pos[3])) {
            append(TOKEN_FOR);
            pos += 3;
        } else if (startsWith(pos, "int") && !isAlphaDigitUnderbar(pos[3])) {
            append(TOKEN_INT);
            pos += 3;
        } else if (isalpha(*pos) || *pos == '_') {
            char* end = pos + 1;
            int len = 1;
            while (isAlphaDigitUnderbar(*end)) {
                ++len;
                ++end;
            }
            append(TOKEN_IDENTIFIER);
            tail->str = strndup(pos, len);
            pos = end;
        } else {
            error_at(pos, source, "トークナイズできません");
        }
    }
    append(TOKEN_EOF);
    return head.next;
}
