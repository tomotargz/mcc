#include <stdio.h>

#include "codegen.h"
#include "error.h"
#include "parse.h"

static int tag = 0;

static void generate(Node* node);

// push the local variable's address to the stack
static void generateAddress(Node* node)
{
    if (node->kind == NODE_LOCAL_VARIABLE) {
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", node->variable->offset);
        printf("  push rax\n");
        return;
    }

    if (node->kind == NODE_GLOBAL_VARIABLE) {
        printf("  push offset %s\n", node->variable->name);
        return;
    }

    if (node->kind == NODE_DEREF) {
        generate(node->lhs);
        return;
    }

    error("invalid node");
}

// generate code that pushes the evaluated value to the top of the stack
static void generate(Node* node)
{
    if (node->kind == NODE_NULL) {
        return;
    } else if (node->kind == NODE_NUMBER) {
        printf("  push %d\n", node->val);
        return;
    } else if (node->kind == NODE_LOCAL_VARIABLE
        || node->kind == NODE_GLOBAL_VARIABLE) {
        generateAddress(node);
        if (node->variable->type->kind == TYPE_ARRAY) {
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
        for (Node* statement = node->statements;
             statement;
             statement = statement->next) {
            generate(statement);
        }
        return;
    } else if (node->kind == NODE_CALL) {
        int t = tag++;
        int argNum = 0;
        for (Node* arg = node->args; arg; arg = arg->next) {
            generate(arg);
            ++argNum;
        }
        static char* ARG_REG[] = {
            "rdi", "rsi", "rdx", "rcx", "r8", "r9"
        };
        for (int i = argNum; i > 0; --i) {
            printf("  pop %s\n", ARG_REG[i - 1]);
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

    if (node->kind == NODE_ADDITION) {
        printf("  add rax, rdi\n");
    } else if (node->kind == NODE_SUBTRACTION) {
        printf("  sub rax, rdi\n");
    } else if (node->kind == NODE_POINTER_ADDITION) {
        if (node->type->kind == TYPE_ARRAY) {
            printf("  imul rdi, %d\n", size(node->type->arrayOf));
        } else {
            printf("  imul rdi, %d\n", size(node->type->pointerTo));
        }
        printf("  add rax, rdi\n");
    } else if (node->kind == NODE_POINTER_SUBTRACTION) {
        if (node->type->kind == TYPE_ARRAY) {
            printf("  imul rdi, %d\n", size(node->type->arrayOf));
        } else {
            printf("  imul rdi, %d\n", size(node->type->pointerTo));
        }
        printf("  sub rax, rdi\n");
    } else if (node->kind == NODE_MULTIPLICATION) {
        printf("  imul rax, rdi\n");
    } else if (node->kind == NODE_DIVISION) {
        printf("  cqo\n");
        printf("  idiv rdi\n");
    } else if (node->kind == NODE_EQUAL) {
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
    } else if (node->kind == NODE_NOT_EQUAL) {
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
    } else if (node->kind == NODE_LESS_THAN) {
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
    } else if (node->kind == NODE_LESS_OR_EQUAL) {
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
    }

    printf("  push rax\n");
}

static char* parameterRegister(int size, int index)
{
    static char* ARG_REG1[] = {
        "al", "sil", "dl", "cl", "r8b", "r9b"
    };
    static char* ARG_REG4[] = {
        "edi", "esi", "edx", "ecx", "r8d", "r9d"
    };
    static char* ARG_REG8[] = {
        "rdi", "rsi", "rdx", "rcx", "r8", "r9"
    };
    if (size == 1) {
        return ARG_REG1[index];
    } else if (size == 4) {
        return ARG_REG4[index];
    } else if (size == 8) {
        return ARG_REG8[index];
    } else {
        error("invalid size");
    }
    return NULL;
}

static void generateFunction(Function* function)
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
    int i = 0;
    int totalParamSize = 0;
    for (; param; param = param->next, ++i) {
        int paramSize = size(param->type);
        totalParamSize += paramSize;
        printf("  sub rsp, %d\n", paramSize);
        printf("  mov [rsp], %s\n", parameterRegister(size(param->type), i));
    }

    // extend stack for local variable
    printf("# extend stack for local variables\n");
    printf("  sub rsp, %d\n", function->stackSize - totalParamSize);

    // generate body
    printf("# generate body\n");
    for (Node* statement = function->statements;
         statement;
         statement = statement->next) {
        generate(statement);
    }

    // epilogue
    printf("# epilogue\n");
    printf(".L.return.%s:\n", function->name);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

static void generateGlobalVariable(Variable* variable)
{
    printf("%s:\n", variable->name);
    if (variable->type->kind == TYPE_ARRAY) {
        printf("  .zero %d\n", variable->type->arraySize * 8);
    } else {
        printf("  .zero %d\n", 8);
    }
}

void generateCode(Program* program)
{
    printf(".intel_syntax noprefix\n");
    printf(".data\n");
    for (Variable* v = program->globalVariables; v; v = v->next) {
        generateGlobalVariable(v);
    }
    printf(".text\n");
    for (Function* f = program->functions; f; f = f->next) {
        generateFunction(f);
    }
}
