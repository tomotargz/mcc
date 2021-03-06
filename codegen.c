#include "mcc.h"

static int tag = 0;
static int loopTag = 0;
static char* functionName;

static void generate(Node* node);

// push the local variable's address to the stack
static void generateAddress(Node* node)
{
    if (node->kind == NODE_LOCAL_VARIABLE) {
        printf("  lea rax, [rbp-%d]\n", node->var->offset);
        printf("  push rax\n");
        return;
    }

    if (node->kind == NODE_MEMBER) {
        generateAddress(node->lhs);
        printf("  pop rax\n");
        printf("  add rax, %d\n", node->member->offset);
        printf("  push rax\n");
        return;
    }

    if (node->kind == NODE_GLOBAL_VARIABLE) {
        printf("  push offset %s\n", node->name);
        return;
    }

    if (node->kind == NODE_DEREF) {
        generate(node->lhs);
        return;
    }

    error("invalid node");
}

static void store(Type* type)
{
    printf("  pop rdi\n");
    printf("  pop rax\n");
    int byte = size(type);
    if (byte == 1) {
        printf("  mov [rax], dil\n");
    } else if (byte == 2) {
        printf("  mov [rax], di\n");
    } else if (byte == 4) {
        printf("  mov [rax], edi\n");
    } else if (byte == 8) {
        printf("  mov [rax], rdi\n");
    }
    printf("  push rdi\n");
}

// Change the top of stack from address to value
static void load(Type* type)
{
    printf("  pop rax\n");
    int byte = size(type);
    if (byte == 1) {
        printf("  movsx rax, byte ptr [rax]\n");
    } else if (byte == 2) {
        printf("  movsx rax, word ptr [rax]\n");
    } else if (byte == 4) {
        printf("  movsxd rax, dword ptr [rax]\n");
    } else if (byte == 8) {
        printf("  mov rax, [rax]\n");
    }
    printf("  push rax\n");
}

static void increment(Type* type)
{
    printf("  pop rax\n");
    if (type->kind == TYPE_POINTER) {
        printf("  add rax, %d\n", size(type->pointerTo));
    } else if (type->kind == TYPE_ARRAY) {
        printf("  add rax, %d\n", size(type->arrayOf));
    } else {
        printf("  add rax, 1\n");
    }
    printf("  push rax\n");
}

static void decrement(Type* type)
{
    printf("  pop rax\n");
    if (type->kind == TYPE_POINTER) {
        printf("  sub rax, %d\n", size(type->pointerTo));
    } else if (type->kind == TYPE_ARRAY) {
        printf("  sub rax, %d\n", size(type->arrayOf));
    } else {
        printf("  sub rax, 1\n");
    }
    printf("  push rax\n");
}

// generate code that pushes the evaluated value to the top of the stack
static void generate(Node* node)
{
    if (node->kind == NODE_NULL) {
    } else if (node->kind == NODE_STATEMENT_EXPRESSION) {
        generate(node->lhs);
        printf("  add rsp, 8\n");
    } else if (node->kind == NODE_NUMBER) {
        printf("  push %d\n", node->val);
    } else if (node->kind == NODE_LOCAL_VARIABLE
        || node->kind == NODE_GLOBAL_VARIABLE
        || node->kind == NODE_MEMBER) {
        generateAddress(node);
        if (node->type->kind != TYPE_ARRAY) {
            load(node->type);
        }
    } else if (node->kind == NODE_ASSIGNMENT) {
        generateAddress(node->lhs);
        generate(node->rhs);
        store(node->type);
    } else if (node->kind == NODE_RETURN) {
        if (node->lhs) {
            generate(node->lhs);
            printf("  pop rax\n");
        }
        printf("  jmp .L.return.%s\n", functionName);
    } else if (node->kind == NODE_BREAK) {
        printf("  jmp .L.break.%d\n", loopTag);
    } else if (node->kind == NODE_CONTINUE) {
        printf("  jmp .L.continue.%d\n", loopTag);
    } else if (node->kind == NODE_IF) {
        int t = ++tag;
        generate(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if (node->els) {
            printf("  je .L.else.%d\n", t);
            generate(node->then);
            printf("  jmp .L.end.%d\n", t);
            printf(".L.else.%d:\n", t);
            generate(node->els);
        } else {
            printf("  je .L.end.%d\n", t);
            generate(node->then);
        }
        printf(".L.end.%d:\n", t);
    } else if (node->kind == NODE_WHILE) {
        int t = ++tag;
        int temp = loopTag;
        loopTag = t;
        printf(".L.continue.%d:\n", t);
        generate(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .L.break.%d\n", t);
        generate(node->body);
        printf("  jmp .L.continue.%d\n", t);
        printf(".L.break.%d:\n", t);
        loopTag = temp;
    } else if (node->kind == NODE_FOR) {
        int t = ++tag;
        int temp = loopTag;
        loopTag = t;
        if (node->init) {
            generate(node->init);
        }
        printf(".L.begin.%d:\n", t);
        if (node->cond) {
            generate(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .L.break.%d\n", t);
        }
        generate(node->body);
        printf(".L.continue.%d:\n", t);
        for (Node* n = node->incs; n; n = n->next) {
            generate(n);
        }
        printf("  jmp .L.begin.%d\n", t);
        printf(".L.break.%d:\n", t);
        loopTag = temp;
    } else if (node->kind == NODE_BLOCK) {
        for (Node* statement = node->statements;
             statement;
             statement = statement->next) {
            generate(statement);
        }
    } else if (node->kind == NODE_CALL) {
        if (!strcmp(node->name, "__builtin_va_start")) {
            printf("  pop rax\n");
            printf("  mov edi, dword ptr [rbp-8]\n");
            printf("  mov dword ptr [rax], 0\n");
            printf("  mov dword ptr [rax+4], 0\n");
            printf("  mov qword ptr [rax+8], rdi\n");
            printf("  mov qword ptr [rax+16], 0\n");
        } else {
            int t = ++tag;
            int argNum = 0;
            for (Node* arg = node->args; arg; arg = arg->next) {
                generate(arg);
                ++argNum;
            }
            char* ARG_REG[] = {
                "rdi", "rsi", "rdx", "rcx", "r8", "r9"
            };
            for (int i = argNum; i > 0; --i) {
                printf("  pop %s\n", ARG_REG[i - 1]);
            }
            // 16byte align
            printf("  mov rax, rsp\n");
            printf("  and rax, 15\n");
            printf("  jnz .L.call.%d\n", t);
            printf("  mov rax, 0\n");
            printf("  call %s\n", node->name);
            printf("  jmp .L.end.%d\n", t);
            printf(".L.call.%d:\n", t);
            printf("  sub rsp, 8\n");
            printf("  mov rax, 0\n");
            printf("  call %s\n", node->name);
            printf("  add rsp, 8\n");
            printf(".L.end.%d:\n", t);
            printf("  push rax\n");
        }
    } else if (node->kind == NODE_ADDR) {
        generateAddress(node->lhs);
    } else if (node->kind == NODE_DEREF) {
        generate(node->lhs);
        load(node->type);
    } else if (node->kind == NODE_PRE_INCREMENT) {
        generateAddress(node->lhs);
        printf("  push [rsp]\n");
        load(node->type);
        increment(node->type);
        store(node->type);
    } else if (node->kind == NODE_POST_INCREMENT) {
        generateAddress(node->lhs);
        printf("  push [rsp]\n");
        load(node->type);
        increment(node->type);
        store(node->type);
        decrement(node->type);
    } else if (node->kind == NODE_PRE_DECREMENT) {
        generateAddress(node->lhs);
        printf("  push [rsp]\n");
        load(node->type);
        decrement(node->type);
        store(node->type);
    } else if (node->kind == NODE_POST_DECREMENT) {
        generateAddress(node->lhs);
        printf("  push [rsp]\n");
        load(node->type);
        decrement(node->type);
        store(node->type);
        increment(node->type);
    } else if (node->kind == NODE_NOT) {
        generate(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");
    } else if (node->kind == NODE_ADD_EQ) {
        generateAddress(node->lhs);
        generate(node->lhs);
        generate(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  add rax, rdi\n");
        printf("  push rax\n");
        store(node->type);
    } else if (node->kind == NODE_ADDITION) {
        generate(node->lhs);
        generate(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  add rax, rdi\n");
        printf("  push rax\n");
    } else if (node->kind == NODE_SUBTRACTION) {
        generate(node->lhs);
        generate(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  sub rax, rdi\n");
        printf("  push rax\n");
    } else if (node->kind == NODE_POINTER_ADDITION) {
        generate(node->lhs);
        generate(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        if (node->type->kind == TYPE_ARRAY) {
            printf("  imul rdi, %d\n", size(node->type->arrayOf));
        } else {
            printf("  imul rdi, %d\n", size(node->type->pointerTo));
        }
        printf("  add rax, rdi\n");
        printf("  push rax\n");
    } else if (node->kind == NODE_POINTER_SUBTRACTION) {
        generate(node->lhs);
        generate(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        if (node->type->kind == TYPE_ARRAY) {
            printf("  imul rdi, %d\n", size(node->type->arrayOf));
        } else {
            printf("  imul rdi, %d\n", size(node->type->pointerTo));
        }
        printf("  sub rax, rdi\n");
        printf("  push rax\n");
    } else if (node->kind == NODE_MULTIPLICATION) {
        generate(node->lhs);
        generate(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  imul rax, rdi\n");
        printf("  push rax\n");
    } else if (node->kind == NODE_DIVISION) {
        generate(node->lhs);
        generate(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cqo\n");
        printf("  idiv rdi\n");
        printf("  push rax\n");
    } else if (node->kind == NODE_EQUAL) {
        generate(node->lhs);
        generate(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");
    } else if (node->kind == NODE_NOT_EQUAL) {
        generate(node->lhs);
        generate(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");
    } else if (node->kind == NODE_LESS_THAN) {
        generate(node->lhs);
        generate(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");
    } else if (node->kind == NODE_LESS_OR_EQUAL) {
        generate(node->lhs);
        generate(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");
    } else if (node->kind == NODE_AND) {
        int t = ++tag;
        generate(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .L.false.%d\n", t);
        generate(node->rhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .L.false.%d\n", t);
        printf("  push 1\n");
        printf("  jmp .L.end.%d\n", t);
        printf(".L.false.%d:\n", t);
        printf("  push 0\n");
        printf(".L.end.%d:\n", t);
    } else if (node->kind == NODE_OR) {
        int t = ++tag;
        generate(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  jne .L.true.%d\n", t);
        generate(node->rhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  jne .L.true.%d\n", t);
        printf("  push 0\n");
        printf("  jmp .L.end.%d\n", t);
        printf(".L.true.%d:\n", t);
        printf("  push 1\n");
        printf(".L.end.%d:\n", t);
    }
}

static char* parameterRegister(int size, int index)
{
    char* ARG_REG1[] = {
        "al", "sil", "dl", "cl", "r8b", "r9b"
    };
    char* ARG_REG2[] = {
        "ax", "si", "dx", "cx", "r8w", "r9w"
    };
    char* ARG_REG4[] = {
        "edi", "esi", "edx", "ecx", "r8d", "r9d"
    };
    char* ARG_REG8[] = {
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
    functionName = function->name;
    if (!function->isStatic) {
        printf(".global %s\n", function->name);
    }
    printf("%s:\n", function->name);

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");

    // Extend stack for local variables
    if (function->stackSize) {
        printf("  sub rsp, %d\n", function->stackSize);
    }

    // Save arg registers if function is variadic
    if (function->isVariadic) {
        int n = 0;
        for (Node* p = function->params; p; p = p->next) {
            n++;
        }
        printf("mov dword ptr [rbp-8], %d\n", n * 8);
        printf("mov [rbp-16], r9\n");
        printf("mov [rbp-24], r8\n");
        printf("mov [rbp-32], rcx\n");
        printf("mov [rbp-40], rdx\n");
        printf("mov [rbp-48], rsi\n");
        printf("mov [rbp-56], rdi\n");
    }

    // copy params to stack
    Node* param = function->params;
    int i = 0;
    for (; param; param = param->next, ++i) {
        printf("  mov [rbp-%d], %s\n", param->var->offset, parameterRegister(size(param->type), i));
    }

    // generate body
    for (Node* statement = function->statements;
         statement;
         statement = statement->next) {
        generate(statement);
    }

    // epilogue
    printf(".L.return.%s:\n", function->name);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

static char* dataType(Type* t)
{
    if (t->kind == TYPE_ARRAY) {
        return dataType(t->arrayOf);
    } else if (t->kind == TYPE_CHAR) {
        return "byte";
    } else if (t->kind == TYPE_SHORT) {
        return "value";
    } else if (t->kind == TYPE_INT) {
        return "long";
    } else if (t->kind == TYPE_LONG
        || t->kind == TYPE_POINTER) {
        return "quad";
    }
    error("invalid type");
    return NULL;
}

static bool initializeGlobalVariable(Variable* v)
{
    if (!v->initialValue) {
        return false;
    }

    if (v->initialValue->label) {
        printf("  .quad %s\n", v->initialValue->label);
        return true;
    }

    if (v->initialValue->string) {
        printf("  .string \"%s\"\n", v->initialValue->string);
        return true;
    }

    ValueList* val = v->initialValue->valueList;
    while (val) {
        printf("  .%s %d\n", dataType(v->type), val->value);
        val = val->next;
    }
    return true;
}

static void generateGlobalVariable(Variable* variable)
{
    printf(".align %d\n", variable->type->align);
    printf("%s:\n", variable->name);

    if (variable->string) {
        for (char* c = variable->string; *c; c++) {
            printf("  .byte %d\n", *c);
        }
        printf("  .byte 0\n");
        return;
    }

    if (initializeGlobalVariable(variable)) {
        return;
    }
    printf("  .zero %d\n", size(variable->type));
}

void generateCode(Program* program)
{
    printf(".intel_syntax noprefix\n");
    printf(".data\n");
    for (VariableList* list = program->globalVariables; list; list = list->next) {
        generateGlobalVariable(list->variable);
    }
    printf(".text\n");
    for (Function* f = program->functions; f; f = f->next) {
        generateFunction(f);
    }
}
