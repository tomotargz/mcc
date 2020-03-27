#include <stdio.h>

#include "codegen.h"
#include "error.h"

static int tag = 0;

void generate_lval(Node* node)
{
    if (node->kind != NODE_LOCAL_VARIABLE) {
        error("node is not local variable");
    }
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void generate(Node* node)
{
    if (node->kind == NODE_NUMBER) {
        printf("  push %d\n", node->val);
        return;
    } else if (node->kind == NODE_LOCAL_VARIABLE) {
        generate_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    } else if (node->kind == NODE_ASSIGNMENT) {
        generate_lval(node->lhs);
        generate(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;
    } else if (node->kind == NODE_RETURN) {
        generate(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    } else if (node->kind == NODE_IF) {
        int t = tag++;
        generate(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if (node->els) {
            printf("  je .Lelse%d\n", t);
            generate(node->then);
            printf("  jmp .Lend%d\n", t);
            printf(".Lelse%d:\n", t);
            generate(node->els);
        } else {
            printf("  je .Lend%d\n", t);
            generate(node->then);
        }
        printf(".Lend%d:\n", t);
        return;
    } else if (node->kind == NODE_WHILE) {
        int t = tag++;
        printf(".Lbegin%d:\n", t);
        generate(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%d\n", t);
        generate(node->body);
        printf(" jmp .Lbegin%d\n", t);
        printf(".Lend%d:\n", t);
        return;
    } else if (node->kind == NODE_FOR) {
        int t = tag++;
        if (node->init) {
            generate(node->init);
        }
        printf(".Lbegin%d:\n", t);
        if (node->cond) {
            generate(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend%d\n", t);
        }
        generate(node->body);
        if (node->inc){
            generate(node->inc);
        }
        printf("  jmp .Lbegin%d\n", t);
        printf(".Lend%d:\n", t);
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
