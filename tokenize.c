#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "tokenize.h"

static bool startsWith(char* a, char* b)
{
    return memcmp(a, b, strlen(b)) == 0;
}

static Token* newToken(TokenKind kind)
{
    Token* token = calloc(1, sizeof(Token));
    token->kind = kind;
    return token;
}

static Token* append(Token* tail, Token* token)
{
    tail->next = token;
    return token;
}

static char* startsWithReserved(char* str)
{
    static char* KEYWORD[] = {
        "return", "if", "else", "while", "for", "int", "char", "sizeof"
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

char* extractStringLiteral(char* src)
{
    int length = 0;
    char* rp = src;
    while (!(*rp == '"' && *(rp - 1) != '\\')) {
        length++;
        rp++;
    }
    char* str = calloc(length + 1, sizeof(char));
    strncpy(str, src, length);
    str[length] = '\0';
    return str;
}

char* decodeEscape(char* str)
{
    char* r = str;
    char* w = str;
    while (*r) {
        if (*r == '\\') {
            if (*(r + 1) == 'n') {
                *w = '\n';
            } else if (*(r + 1) == '"') {
                *w = '"';
            } else {
                error("unsupported escape character");
            }
            w++;
            r += 2;
            continue;
        }
        *w = *r;
        r++;
        w++;
    }
    *w = '\0';
    return str;
}

Token* tokenize(char* source)
{
    Token dummy;
    Token* tail = &dummy;
    char* rp = source;
    while (*rp) {
        if (startsWith(rp, "/*")) {
            rp += 2;
            while (!startsWith(rp, "*/")) {
                rp++;
            }
            rp += 2;
            continue;
        }

        if (startsWith(rp, "//")) {
            while (*rp != '\n') {
                rp++;
            }
            rp++;
            continue;
        }

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
            token->str = calloc(length, sizeof(char));
            strncpy(token->str, start, length);
            tail = append(tail, token);
            continue;
        }

        if (isdigit(*rp)) {
            Token* token = newToken(TOKEN_NUMBER);
            token->val = strtol(rp, &rp, 10);
            tail = append(tail, token);
            continue;
        }

        if (*rp == '"') {
            rp++;
            char* str = extractStringLiteral(rp);
            rp += strlen(str) + 1;
            str = decodeEscape(str);
            Token* token = newToken(TOKEN_STRING);
            token->str = str;
            tail = append(tail, token);
            continue;
        }
        error_at(rp, source, "Can't tokenize");
    }
    Token* token = newToken(TOKEN_EOF);
    tail = append(tail, token);
    return dummy.next;
}
