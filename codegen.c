#include <stdio.h>

#include "codegen.h"

void generate(Node* node)
{
    if (node->kind == NODE_NUMBER) {
        printf("  push %d\n", node->val);
        return;
    }

    generate(node->lhs);
    generate(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
    case NODE_ADDITION:
        printf("  add rax, rdi\n");
        break;
    case NODE_SUBTRACTION:
        printf("  sub rax, rdi\n");
        break;
    case NODE_MULTIPLICATION:
        printf("  imul rax, rdi\n");
        break;
    case NODE_DIVISION:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    case NODE_EQUAL:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case NODE_NOT_EQUAL:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case NODE_LESS_THAN:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case NODE_LESS_OR_EQUAL:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    default:
        break;
    }

    printf("  push rax\n");
}
