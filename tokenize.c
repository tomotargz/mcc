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

Token* newToken(TokenKind kind)
{
    Token* token = calloc(1, sizeof(Token));
    token->kind = kind;
    return token;
}

Token* append(Token* tail, Token* token)
{
    tail->next = token;
    return token;
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
    Token dummy;
    Token* tail = &dummy;
    char* rp = source;
    while (*rp) {
        if (isspace(*rp)) {
            rp++;
            continue;
        }

        char* reserved = startsWithReserved(rp);
        if (reserved) {
            Token* token = newToken(TOKEN_RESERVED);
            token->str = reserved;
            tail = append(tail, token);
            rp += strlen(reserved);
            continue;
        }

        if (isalpha(*rp) || *rp == '_') {
            char* start = rp;
            int length = 0;
            while (isalnum(*rp) || *rp == '_') {
                length++;
                rp++;
            }
            Token* token = newToken(TOKEN_IDENTIFIER);
            token->str = strndup(start, length);
            tail = append(tail, token);
            continue;
        }

        if (isdigit(*rp)) {
            Token* token = newToken(TOKEN_NUMBER);
            token->val = strtol(rp, &rp, 10);
            tail = append(tail, token);
            continue;
        }
        error_at(rp, source, "Can't tokenize");
    }
    Token* token = newToken(TOKEN_EOF);
    tail = append(tail, token);
    return dummy.next;
}
