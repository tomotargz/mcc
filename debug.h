#ifndef DEBUG_H
#define DEBUG_H

#include "tokenize.h"
#include "error.h"

void printTokens(Token* tokens)
{
    for (Token* crr = tokens; crr; crr = crr->next) {
        if (crr->kind == TOKEN_RESERVED) {
            info("reserved %.*s", crr->length, crr->str);
        } else if (crr->kind == TOKEN_IDENTIFIER) {
            info("identifier %.*s", crr->length, crr->str);
        } else if (crr->kind == TOKEN_NUMBER) {
            info("number %d", crr->value);
        } else if (crr->kind == TOKEN_EOF) {
            info("eof");
        }
    }
}

#endif
