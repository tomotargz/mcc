#ifndef DEBUG_H
#define DEBUG_H

#include "error.h"
#include "tokenize.h"

void printTokens(Token* tokens)
{
    for (Token* crr = tokens; crr; crr = crr->next) {
        if (crr->kind == TOKEN_RESERVED) {
            info("reserved %.*s", crr->len, crr->str);
        } else if (crr->kind == TOKEN_IDENTIFIER) {
            info("identifier %.*s", crr->len, crr->str);
        } else if (crr->kind == TOKEN_NUMBER) {
            info("number %d", crr->val);
        } else if (crr->kind == TOKEN_RETURN) {
            info("return");
        } else if (crr->kind == TOKEN_IF) {
            info("if");
        } else if (crr->kind == TOKEN_ELSE) {
            info("else");
        } else if (crr->kind == TOKEN_WHILE) {
            info("while");
        } else if (crr->kind == TOKEN_EOF) {
            info("eof");
        } else {
            info("%c", crr->kind);
        }
    }
}

#endif
