#include <stdio.h>

#include "codegen.h"
#include "error.h"
#include "parse.h"

static int tag = 0;

static void generate(Node* node);

// push the local variable's address to the stack
void generateAddress(Node* node)
{
    if (node->kind == NODE_LOCAL_VARIABLE) {
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", node->localVariable->offset);
        printf("  push rax\n");
        return;
    }

    if (node->kind == NODE_DEREF) {
        generate(node->lhs);
        return;
    }

    error("invalid node");
}

// generate code that pushes the evaluated value to the top of the stack
void generate(Node* node)
{
    if (node->kind == NODE_NULL) {
        return;
    } else if (node->kind == NODE_NUMBER) {
        printf("  push %d\n", node->val);
        return;
    } else if (node->kind == NODE_LOCAL_VARIABLE) {
        generateAddress(node);
        if (node->localVariable->type->kind == TYPE_ARRAY) {
            return;
        }
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    } else if (node->kind == NODE_ASSIGNMENT) {
        generateAddress(node->lhs);
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
        if (node->inc) {
            generate(node->inc);
        }
        printf("  jmp .Lbegin%d\n", t);
        printf(".Lend%d:\n", t);
        return;
    } else if (node->kind == NODE_BLOCK) {
        for (int i = 0; node->statements[i]; ++i) {
            generate(node->statements[i]);
        }
        return;
    } else if (node->kind == NODE_CALL) {
        int t = tag++;
        int i = 0;
        for (; node->args[i] != NULL; ++i) {
            generate(node->args[i]);
        }
        --i;
        static char* ARG_REG[] = {
            "rdi", "rsi", "rdx", "rcx", "r8", "r9"
        };
        for (; i >= 0; --i) {
            printf("  pop %s\n", ARG_REG[i]);
        }
        // 16byte align
        printf("  mov rax, rsp\n");
        printf("  and rax, 15\n");
        printf("  jnz .Lalign%d\n", t);
        printf("  mov rax, 0\n");
        printf("  call %s\n", node->name);
        printf("  jmp .Lend%d\n", t);
        printf(".Lalign%d:\n", t);
        printf("  sub rsp, 8\n");
        printf("  mov rax, 0\n");
        printf("  call %s\n", node->name);
        printf("  add rsp, 8\n");
        printf(".Lend%d:\n", t);
        printf("  push rax\n");
        return;
    } else if (node->kind == NODE_ADDR) {
        generateAddress(node->lhs);
        return;
    } else if (node->kind == NODE_DEREF) {
        generate(node->lhs);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
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
    case NODE_POINTER_ADDITION:
        if (pointee(node->lhs->type) == TYPE_INT) {
            printf("  imul rdi, 8\n");
        } else {
            printf("  imul rdi, 8\n");
        }
        printf("  add rax, rdi\n");
        break;
    case NODE_POINTER_SUBTRACTION:
        if (pointee(node->lhs->type) == TYPE_INT) {
            printf("  imul rdi, 8\n");
        } else {
            printf("  imul rdi, 8\n");
        }
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

int size(Node* node)
{
    if (node->type->kind == TYPE_INT) {
        return 8;
    } else if (node->type->kind == TYPE_POINTER) {
        return 8;
    }
    error("unexpected type");
    return 8;
}

void generateFunction(Function* function)
{
    printf(".global %s\n", function->name);
    printf("%s:\n", function->name);

    // prologue
    printf("# prolugue\n");
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");

    // copy params to stack
    printf("# push params to stack\n");
    Node* param = function->params;
    static char* ARG_REG[] = {
        "rdi", "rsi", "rdx", "rcx", "r8", "r9"
    };
    int i = 0;
    int totalParamSize = 0;
    for (; param; param = param->next, ++i) {
        int paramSize = size(param);
        totalParamSize += paramSize;
        printf("  sub rsp, %d\n", paramSize);
        printf("  mov [rsp], %s\n", ARG_REG[i]);
    }

    // extend stack for local variable
    printf("# extend stack for local variables\n");
    printf("  sub rsp, %d\n", function->stackSize - totalParamSize);

    // generate body
    printf("# generate body\n");
    for (Node* n = function->node; n; n = n->next) {
        generate(n);
    }

    // epilogue
    printf("# epilogue\n");
    printf(".L.return.%s:\n", function->name);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

void generateFunctions(Function* functions)
{
    printf(".intel_syntax noprefix\n");

    for (Function* function = functions; function; function = function->next) {
        generateFunction(function);
    }
}
