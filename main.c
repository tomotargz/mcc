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
    ParseResult result = parse(tokens);
    Node** ast = result.ast;
    LVar* lvars = result.lvars;
    int offset = 0;
    for (; lvars; lvars = lvars->next) {
        if(!lvars->next){
            offset = lvars->offset;
            break;
        }
    }

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", offset);

    // 抽象構文木を下りながらコード生成
    while (*ast) {
        generate(*ast);
        printf("  pop rax\n");
        ++ast;
    }

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}
