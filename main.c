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
        printf("# prolugue\n");
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");

        // copy params to stack
        printf("# push params to stack\n");
        Node* param = f->params;
        static char* ARG_REG[] = {
            "rdi", "rsi", "rdx", "rcx", "r8", "r9"
        };
        int i = 0;
        for (; param; param = param->next, ++i) {
            printf("  sub rsp, 8\n");
            printf("  mov [rsp], %s\n", ARG_REG[i]);
        }

        // extend stack for local variable
        printf("# extend stack for local variables\n");
        printf("  sub rsp, %d\n", f->stackSize - i * 8);

        // generate body
        printf("# generate body\n");
        for (Node* n = f->node; n; n = n->next) {
            generate(n);
        }

        // epilogue
        printf("# epilogue\n");
        printf(".L.return.%s:\n", f->name);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }
    return 0;
}
