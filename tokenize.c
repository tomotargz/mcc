#include "mcc.h"

static bool startsWith(char* a, char* b)
{
    return memcmp(a, b, strlen(b)) == 0;
}

static Token* newToken(TokenKind kind, char* pos)
{
    Token* token = calloc(1, sizeof(Token));
    token->kind = kind;
    token->pos = pos;
    return token;
}

static Token* append(Token* tail, Token* token)
{
    tail->next = token;
    return token;
}

static char* startsWithReserved(char* str)
{
    char* KEYWORD[] = {
        "return", "if", "else", "while", "for", "break", "continue",
        "void", "char", "short", "int", "long", "struct", "enum",
        "typedef", "extern", "static", "sizeof"
    };
    for (int i = 0; i < sizeof(KEYWORD) / sizeof(KEYWORD[0]); ++i) {
        int length = strlen(KEYWORD[i]);
        if (startsWith(str, KEYWORD[i])
            && !isalnum(str[length])
            && str[length] != '_') {
            return KEYWORD[i];
        }
    }

    char* PUNCTUATION[] = {
        "...",
        "==", "!=", "<=", ">=", "->", "<", ">",
        "++", "--", "+=",
        "&&", "||",
        "=", "+", "-", "*", "/",
        "(", ")", "{", "}", "[", "]",
        ";", ",", "&", ".", "!"
    };
    for (int i = 0; i < sizeof(PUNCTUATION) / sizeof(PUNCTUATION[0]); ++i) {
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
            } else if (*(r + 1) == '\'') {
                *w = '\'';
            } else if (*(r + 1) == '\\') {
                *w = '\\';
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

Token* tokenize(char* source, char* file)
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
            Token* token = newToken(TOKEN_RESERVED, rp);
            token->str = reserved;
            tail = append(tail, token);
            rp += strlen(reserved);
            continue;
        }

        if (isalpha(*rp) || *rp == '_') {
            Token* token = newToken(TOKEN_IDENTIFIER, rp);
            char* start = rp;
            int length = 0;
            while (isalnum(*rp) || *rp == '_') {
                length++;
                rp++;
            }
            token->str = calloc(length, sizeof(char));
            strncpy(token->str, start, length);
            tail = append(tail, token);
            continue;
        }

        if (isdigit(*rp)) {
            Token* token = newToken(TOKEN_NUMBER, rp);
            token->val = strtol(rp, &rp, 10);
            tail = append(tail, token);
            continue;
        }

        if (*rp == '"') {
            Token* token = newToken(TOKEN_STRING, rp);
            rp++;
            char* str = extractStringLiteral(rp);
            rp += strlen(str) + 1;
            str = decodeEscape(str);
            token->str = str;
            tail = append(tail, token);
            continue;
        }

        if (*rp == '\'') {
            ++rp;
            Token* token = newToken(TOKEN_NUMBER, rp);
            if (*rp == '\\') {
                ++rp;
                if (*rp == 'n') {
                    token->val = '\n';
                } else if (*rp == '0') {
                    token->val = '\0';
                } else if (*rp == '\\') {
                    token->val = '\\';
                } else if (*rp == '\'') {
                    token->val = '\'';
                } else if (*rp == '"') {
                    token->val = '"';
                } else {
                    error_at(rp, source, file, "unsupported escape char");
                }
            } else {
                token->val = *rp;
            }
            tail = append(tail, token);
            ++rp;
            if (*rp != '\'') {
                error_at(rp, source, file, "expect '");
            }
            ++rp;
            continue;
        }

        error_at(rp, source, file, "Can't tokenize");
    }
    Token* token = newToken(TOKEN_EOF, rp);
    tail = append(tail, token);
    return dummy.next;
}
