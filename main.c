#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "debug.h"
#include "error.h"
#include "parse.h"
#include "tokenize.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    char* source = argv[1];
    Token* tokens = tokenize(source);
    // printTokens(tokens);
    Function* func = parse(tokens);

    printf(".intel_syntax noprefix\n");

    for (Function* f = func; f; f = f->next) {
        printf(".global %s\n", f->name);
        printf("%s:\n", f->name);

        // prologue
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", f->stackSize);

        for (Node* n = func->node; n; n = n->next) {
            generate(n);
        }

        // epilogue
        printf(".L.return.%s:\n", f->name);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }
    return 0;
}
