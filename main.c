#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "error.h"
#include "parse.h"
#include "tokenize.h"
#include "debug.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    char* source = argv[1];
    Token* tokens = tokenize(source);
    printTokens(tokens);
    Node** node = parse(tokens);

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    // 抽象構文木を下りながらコード生成
    while(*node){
        generate(*node);
        printf("  pop rax\n");
        ++node;
    }

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}
