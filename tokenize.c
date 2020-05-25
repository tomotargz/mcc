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

void append(int kind)
{
    Token* t = calloc(1, sizeof(Token));
    t->kind = kind;
    tail->next = t;
    tail = t;
}

char* startsWithReserved(char* str)
{
    static char* KEYWORD[] = {
        "return", "if", "else", "while", "for", "int", "sizeof"
    };
    for (int i = 0; i < sizeof(KEYWORD) / sizeof(KEYWORD[0]); ++i) {
        int length = strlen(KEYWORD[i]);
        if (startsWith(str, KEYWORD[i])
            && !isalnum(str[length])
            && str[length] != '_') {
            return KEYWORD[i];
        }
    }

    static char* PUNCTUATION[] = {
        "==", "!=", "<=", ">=", "<", ">",
        "=", "+", "-", "*", "/",
        "(", ")", "{", "}", "[", "]",
        ";", ",", "&"
    };
    for (int i = 0; i < sizeof(PUNCTUATION) / sizeof(PUNCTUATION[0]); ++i) {
        int length = strlen(PUNCTUATION[i]);
        if (startsWith(str, PUNCTUATION[i])) {
            return PUNCTUATION[i];
        }
    }
    return NULL;
}

Token* tokenize(char* source)
{
    char* pos = source;
    while (*pos) {
        if (isspace(*pos)) {
            pos++;
            continue;
        }

        char* reserved = startsWithReserved(pos);
        if (reserved) {
            append(TOKEN_RESERVED);
            tail->str = reserved;
            pos += strlen(reserved);
            continue;
        }

        if (isalpha(*pos) || *pos == '_') {
            char* start = pos;
            int len = 0;
            while (isalnum(*pos) || *pos == '_') {
                len++;
                pos++;
            }
            append(TOKEN_IDENTIFIER);
            tail->str = strndup(start, len);
            continue;
        }

        if (isdigit(*pos)) {
            append(TOKEN_NUMBER);
            tail->val = strtol(pos, &pos, 10);
            continue;
        }
        error_at(pos, source, "Can't tokenize");
    }
    append(TOKEN_EOF);
    return head.next;
}
