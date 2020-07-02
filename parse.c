#include "mcc.h"

typedef struct StructTag {
    char* name;
    Type* type;
    struct StructTag* next;
} StructTag;

typedef struct Typedef {
    char* name;
    Type* type;
    struct Typedef* next;
} Typedef;

typedef struct VariableScope {
    struct VariableScope* next;
    char* name;
    Variable* variable; // variable is null when it's an enum or a function.
    int enumValue;
    Type* retType; // function's return type
} VariableScope;

typedef struct Scope {
    StructTag* structTags;
    VariableScope* variables;
    Typedef* typedefs;
} Scope;

static VariableScope* variableScope;
static StructTag* structTagScope;
static Typedef* typedefScope;

static char* src;
static char* file;
static Token* rp = NULL;

static VariableList* globalVariables;
static VariableList* localVariables;

static Node* expression();
static Type* basetype(bool* isTypedef);
static Node* newAdd(Node* lhs, Node* rhs);
static Node* newSub(Node* lhs, Node* rhs);
static Variable* declareGlobalVariable(Type* type, char* name);
static Node* statement();
static bool isTypeName();
static Type* declarator(Type* t, char** name);

static void errorMsg(char* str)
{
    error_at(rp->pos, src, file, str);
}

static Scope* saveScope()
{
    Scope* s = calloc(1, sizeof(Scope));
    s->structTags = structTagScope;
    s->variables = variableScope;
    s->typedefs = typedefScope;
    return s;
}

static void restoreScope(Scope* s)
{
    structTagScope = s->structTags;
    variableScope = s->variables;
    typedefScope = s->typedefs;
}

static void addVarToVarScope(Variable* v)
{
    VariableScope* s = calloc(1, sizeof(VariableScope));
    s->name = v->name;
    s->variable = v;
    s->next = variableScope;
    variableScope = s;
}

static void addFuncToVarScope(char* name, Type* retType)
{
    VariableScope* s = calloc(1, sizeof(VariableScope));
    s->name = name;
    s->retType = retType;
    s->next = variableScope;
    variableScope = s;
}

static void addEnumToVarScope(char* name, int val)
{
    VariableScope* s = calloc(1, sizeof(VariableScope));
    s->name = name;
    s->enumValue = val;
    s->next = variableScope;
    variableScope = s;
}

static Token* consume(char* str)
{
    if (rp->kind != TOKEN_RESERVED
        || strlen(rp->str) != strlen(str)
        || strncmp(rp->str, str, strlen(rp->str))) {
        return NULL;
    }
    Token* result = rp;
    rp = rp->next;
    return result;
}

static char* consumeString()
{
    if (rp->kind == TOKEN_STRING) {
        char* string = rp->str;
        rp = rp->next;
        return string;
    }
    return NULL;
}

static void expect(char* str)
{
    if (rp->kind != TOKEN_RESERVED
        || strlen(rp->str) != strlen(str)
        || strncmp(rp->str, str, strlen(rp->str))) {
        errorMsg("unexpected token");
    }
    rp = rp->next;
}

static int expectNumber()
{
    if (rp->kind != TOKEN_NUMBER)
        errorMsg("unexpected non number token");
    int val = rp->val;
    rp = rp->next;
    return val;
}

static char* expectIdentifier()
{
    if (rp->kind != TOKEN_IDENTIFIER) {
        errorMsg("unexpected non identifier token");
    }
    char* name = rp->str;
    rp = rp->next;
    return name;
}

static bool peek(char* str)
{
    return rp->kind == TOKEN_RESERVED
        && strlen(rp->str) == strlen(str)
        && strncmp(rp->str, str, strlen(rp->str)) == 0;
}

static VariableScope* findVariable(char* str)
{
    for (VariableScope* v = variableScope; v; v = v->next) {
        if (strlen(str) == strlen(v->name)
            && strncmp(str, v->name, strlen(str)) == 0) {
            return v;
        }
    }
    return NULL;
}

static char* consumeIdentifier()
{
    if (rp->kind == TOKEN_IDENTIFIER) {
        Token* identifier = rp;
        rp = rp->next;
        return identifier->str;
    }
    return NULL;
}

// arguments = "(" (expression ("," expression)*)? ")"
static Node* arguments()
{
    Node dummy = {};
    Node* tail = &dummy;
    while (!consume(")")) {
        tail->next = expression();
        tail = tail->next;
        consume(",");
    }
    return dummy.next;
}

static char* stringLabel()
{
    static int tag = 0;
    char* label = calloc(20, sizeof(char));
    sprintf(label, "str.literal.%d", tag++);
    return label;
}

static Node* expressionStatement()
{
    Scope* currentScope = saveScope();
    Node* node = newNode(NODE_BLOCK, NULL, NULL);
    Node dummy = {};
    Node* prev = NULL;
    Node* curr = &dummy;
    while (!consume("}")) {
        curr->next = statement();
        prev = curr;
        curr = curr->next;
    }
    if (curr->kind != NODE_STATEMENT_EXPRESSION) {
        errorMsg("expression statement must end with statement expression");
    }
    prev->next = curr->lhs;
    node->statements = dummy.next;
    expect(")");
    restoreScope(currentScope);
    return node;
}

// primary = "(" "{" expressionStatement "}" ")"
//         | "(" expression ")"
//         | identifier arguments?
//         | number
//         | string
static Node* primary()
{
    if (consume("(")) {
        if (consume("{")) {
            Node* node = expressionStatement();
            return node;
        }
        Node* node = expression();
        expect(")");
        return node;
    }

    if (rp->kind == TOKEN_NUMBER) {
        return newNodeNum(expectNumber());
    }

    char* identifier = consumeIdentifier();
    if (identifier) {
        // Function call
        if (consume("(")) {
            Node* node = newNode(NODE_CALL, NULL, NULL);
            node->name = identifier;
            node->args = arguments();
            addType(node);
            VariableScope* s = findVariable(identifier);
            if (s) {
                node->type = s->retType;
            } else {
                info("implicit function declaration: %s", identifier);
                node->type = intType();
            }
            return node;
        }
        // Variable
        VariableScope* v = findVariable(identifier);
        if (v->variable) {
            return newNodeVariable(v->variable);
        }
        // Enum
        return newNodeNum(v->enumValue);
    }

    char* str = consumeString();
    if (str) {
        char* label = stringLabel();
        Type* type = arrayOf(charType(), strlen(rp->str));
        Variable* v = declareGlobalVariable(type, label);
        v->string = str;
        return newNodeVariable(v);
    }

    errorMsg("invalid token.");
    return NULL;
}

static Member* findMember(Type* type, char* name)
{
    if (type->kind != TYPE_STRUCT) {
        errorMsg("attempt to find a member from non struct type");
    }
    for (Member* m = type->members; m; m = m->next) {
        if (strlen(m->name) == strlen(name)
            && strcmp(m->name, name) == 0) {
            return m;
        }
    }
    errorMsg("no such member");
    return NULL;
}

static Node* structRef(Node* node)
{
    addType(node);
    Member* m = findMember(node->type, expectIdentifier());
    Node* n = newNode(NODE_MEMBER, node, NULL);
    n->member = m;
    return n;
}

// postfix = primary ("[" expression "]" | "." identifier | "->" identifier | "++" | "--")*
static Node* postfix()
{
    Node* node = primary();
    while (1) {
        if (consume("[")) {
            Node* index = expression();
            node = newAdd(node, index);
            node = newNode(NODE_DEREF, node, NULL);
            expect("]");
        } else if (consume(".")) {
            node = structRef(node);
        } else if (consume("->")) {
            // a->b is (*a).b
            node = newNode(NODE_DEREF, node, NULL);
            node = structRef(node);
        } else if (consume("++")) {
            node = newNode(NODE_POST_INCREMENT, node, NULL);
        } else if (consume("--")) {
            node = newNode(NODE_POST_DECREMENT, node, NULL);
        } else {
            break;
        }
    }
    return node;
}

static Node* unary();

// type-suffix = ("[" num? "]")*
Type* typeSuffix(Type* t)
{
    while (consume("[")) {
        if (rp->kind == TOKEN_NUMBER) {
            t = arrayOf(t, expectNumber());
        } else {
            t = arrayOf(t, 0);
        }
        expect("]");
    }
    return t;
}

// abstractDeclarator = "*"* ("(" abstractDeclarator ")")?  typeSuffix?
Type* abstractDeclarator(Type* t)
{
    while (consume("*")) {
        t = pointerTo(t);
    }
    if (consume("(")) {
        Type* placeholder = calloc(1, sizeof(Type));
        Type* newType = abstractDeclarator(placeholder);
        expect(")");
        t = typeSuffix(t);
        memcpy(placeholder, t, sizeof(Type));
        return newType;
    }
    t = typeSuffix(t);
    return t;
}

// typeName = basetype abstract-declarator type-suffix
Type* typeName()
{
    Type* t = basetype(NULL);
    t = abstractDeclarator(t);
    t = typeSuffix(t);
    return t;
}

// unary = ("+" | "-" | "*" | "&" | "!")? unary
//       | postfix
//       | "++" unary
//       | "--" unary
//       | "sizeof" unary
//       | "sizeof" "(" typeName ")"
static Node* unary()
{
    if (consume("+")) {
        return unary();
    } else if (consume("-")) {
        return newNode(NODE_SUBTRACTION, newNodeNum(0), unary());
    } else if (consume("&")) {
        return newNode(NODE_ADDR, unary(), NULL);
    } else if (consume("*")) {
        return newNode(NODE_DEREF, unary(), NULL);
    } else if (consume("!")) {
        return newNode(NODE_NOT, unary(), NULL);
    } else if (consume("++")) {
        return newNode(NODE_PRE_INCREMENT, unary(), NULL);
    } else if (consume("--")) {
        return newNode(NODE_PRE_DECREMENT, unary(), NULL);
    } else if (consume("sizeof")) {
        if (consume("(")) {
            if (isTypeName()) {
                Type* t = typeName();
                expect(")");
                return newNodeNum(size(t));
            }
            Node* node = unary();
            addType(node);
            expect(")");
            return newNodeNum(size(node->type));
        }
        Node* node = unary();
        addType(node);
        return newNodeNum(size(node->type));
    }
    return postfix();
}

// multiplication = unary ("*" unary | "/" unary)*
static Node* mul()
{
    Node* node = unary();
    for (;;) {
        if (consume("*")) {
            node = newNode(NODE_MULTIPLICATION, node, unary());
        } else if (consume("/")) {
            node = newNode(NODE_DIVISION, node, unary());
        } else {
            return node;
        }
    }
}

static Node* newAdd(Node* lhs, Node* rhs)
{
    addType(lhs);
    addType(rhs);
    TypeKind lType = lhs->type->kind;
    TypeKind rType = rhs->type->kind;
    if ((lType == TYPE_INT || lType == TYPE_CHAR)
        && (rType == TYPE_INT || rType == TYPE_CHAR)) {
        return newNode(NODE_ADDITION, lhs, rhs);
    }
    if ((lType == TYPE_POINTER || lType == TYPE_ARRAY)
        && (rType == TYPE_INT || rType == TYPE_CHAR)) {
        return newNode(NODE_POINTER_ADDITION, lhs, rhs);
    }
    errorMsg("invalid addition");
    return NULL;
}

static Node* newSub(Node* lhs, Node* rhs)
{
    addType(lhs);
    addType(rhs);
    TypeKind lType = lhs->type->kind;
    TypeKind rType = rhs->type->kind;
    if ((lType == TYPE_INT || lType == TYPE_CHAR)
        && (rType == TYPE_INT || rType == TYPE_CHAR)) {
        return newNode(NODE_SUBTRACTION, lhs, rhs);
    }
    if ((lType == TYPE_POINTER || lType == TYPE_ARRAY)
        && (rType == TYPE_INT || rType == TYPE_CHAR)) {
        return newNode(NODE_POINTER_SUBTRACTION, lhs, rhs);
    }
    errorMsg("invalid subtraction");
    return NULL;
}

// add = multiplication ("+" multiplication | "-" multiplication)*
static Node* add()
{
    Node* node = mul();
    for (;;) {
        if (consume("+")) {
            node = newAdd(node, mul());
        } else if (consume("-")) {
            node = newSub(node, mul());
        } else {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node* relational()
{
    Node* node = add();
    for (;;) {
        if (consume("<=")) {
            node = newNode(NODE_LESS_OR_EQUAL, node, add());
        } else if (consume("<")) {
            node = newNode(NODE_LESS_THAN, node, add());
        } else if (consume(">=")) {
            node = newNode(NODE_LESS_OR_EQUAL, add(), node);
        } else if (consume(">")) {
            node = newNode(NODE_LESS_THAN, add(), node);
        } else {
            return node;
        }
    }
}

// equality = relational ("==" relational | "!=" relational)*
static Node* equality()
{
    Node* node = relational();
    for (;;) {
        if (consume("==")) {
            node = newNode(NODE_EQUAL, node, relational());
        } else if (consume("!=")) {
            node = newNode(NODE_NOT_EQUAL, node, relational());
        } else {
            return node;
        }
    }
}

// and = equality ("&&" equality)?
static Node* and ()
{
    Node* node = equality();
    if (consume("&&")) {
        node = newNode(NODE_AND, node, equality());
    }
    return node;
}

// or = and ("||" and)?
static Node* or ()
{
    Node* node = and();
    if (consume("||")) {
        node = newNode(NODE_OR, node, and());
    }
    return node;
}

// assign = or ("=" assign)?
static Node* assign()
{
    Node* node = or ();
    if (consume("=")) {
        node = newNode(NODE_ASSIGNMENT, node, assign());
    }
    return node;
}

// expression = assign
static Node* expression()
{
    return assign();
}

static Variable* declareLocalVariable(Type* type, char* name)
{
    Variable* v = calloc(1, sizeof(Variable));
    v->name = name;
    v->type = type;
    v->isGlobal = false;
    if (localVariables) {
        int aligned
            = alignOffset(localVariables->variable->offset, type->align);
        v->offset = aligned + size(type);
    } else {
        v->offset = size(type);
    }
    VariableList* l = calloc(1, sizeof(VariableList));
    l->variable = v;
    l->next = localVariables;
    localVariables = l;
    addVarToVarScope(v);
    return v;
}

static Node* assginToArray(Variable* array, int index, Node* value)
{
    Node* node = newNodeVariable(array);
    node = newAdd(node, newNodeNum(index));
    node = newNode(NODE_DEREF, node, NULL);
    node = newNode(NODE_ASSIGNMENT, node, value);
    return newNode(NODE_STATEMENT_EXPRESSION, node, NULL);
}

// localVariableInitializer = expression | "{" (expression ("," expression)*)? "}"
static Node* localVariableInitializer(Variable* v)
{
    if (v->type->kind == TYPE_ARRAY) {
        int i = 0;
        Node* block = newNode(NODE_BLOCK, NULL, NULL);
        char* str = consumeString();
        if (str) {
            int len = strlen(str);
            for (; i < len; i++) {
                Node* node = assginToArray(v, i, newNodeNum(str[i]));
                node->next = block->statements;
                block->statements = node;
            }
            Node* node = assginToArray(v, i++, newNodeNum('\0'));
            node->next = block->statements;
            block->statements = node;
        } else {
            expect("{");
            for (; !consume("}"); i++) {
                Node* node = assginToArray(v, i, expression());
                node->next = block->statements;
                block->statements = node;
                consume(",");
            }
        }
        if (v->type->arraySize == 0) {
            v->type->arraySize = i;
            return block;
        }
        for (; i < v->type->arraySize; i++) {
            Node* node = assginToArray(v, i, newNodeNum(0));
            node->next = block->statements;
            block->statements = node;
        }
        return block;
    }
    Node* node = newNodeVariable(v);
    node = newNode(NODE_ASSIGNMENT, node, expression());
    node = newNode(NODE_STATEMENT_EXPRESSION, node, NULL);
    return node;
}

// localVariable = basetype declarator typeSuffix ("=" localVariableInitializer)? ";"
//               | basetype ";"
static Node* localVariable()
{
    bool isTypedef;
    Type* t = basetype(&isTypedef);
    if (consume(";")) {
        return newNode(NODE_NULL, NULL, NULL);
    }
    char* name = NULL;
    t = declarator(t, &name);
    t = typeSuffix(t);
    if (isTypedef) {
        Typedef* tdef = calloc(1, sizeof(Typedef));
        tdef->name = name;
        tdef->type = t;
        tdef->next = typedefScope;
        typedefScope = tdef;
        expect(";");
        return newNode(NODE_NULL, NULL, NULL);
    }

    if (t == TYPE_VOID) {
        errorMsg("declare void type variable");
    }

    Variable* v = declareLocalVariable(t, name);
    if (consume("=")) {
        Node* node = localVariableInitializer(v);
        expect(";");
        return node;
    }
    expect(";");
    return newNode(NODE_NULL, NULL, NULL);
}

// statementExpression = expression
static Node* statementExpression()
{
    Node* node = newNode(NODE_STATEMENT_EXPRESSION, expression(), NULL);
    return node;
}

static Type* findTypedef(char* name)
{
    for (Typedef* t = typedefScope; t; t = t->next) {
        if (strlen(name) == strlen(t->name) && !strcmp(name, t->name)) {
            return t->type;
        }
    }
    return NULL;
}

static bool isTypeName()
{
    return peek("void")
        || peek("char")
        || peek("short")
        || peek("int")
        || peek("long")
        || peek("struct")
        || peek("enum")
        || peek("typedef")
        || (rp->kind == TOKEN_IDENTIFIER && findTypedef(rp->str));
}

// statement = "return" expression? ";"
//           | "if" "(" expression ")" statement ("else" statement)?
//           | "while" "(" expression ")" statement
//           | "for" "(" (localVariable | statementExpression)? ";" expression? ";" statementExpression? ")" statement
//           | "{" statement* "}"
//           | localVariable
//           | statementExpression ";"
static Node* statement()
{
    Node* node = NULL;
    if (isTypeName()) {
        node = localVariable();
    } else if (consume("return")) {
        if (consume(";")) {
            node = newNode(NODE_RETURN, NULL, NULL);
        } else {
            node = newNode(NODE_RETURN, expression(), NULL);
            expect(";");
        }
    } else if (consume("if")) {
        node = newNode(NODE_IF, NULL, NULL);
        expect("(");
        node->cond = expression();
        expect(")");
        node->then = statement();
        if (consume("else")) {
            node->els = statement();
        } else {
            node->els = NULL;
        }
    } else if (consume("while")) {
        node = newNode(NODE_WHILE, NULL, NULL);
        expect("(");
        node->cond = expression();
        expect(")");
        node->body = statement();
    } else if (consume("for")) {
        node = newNode(NODE_FOR, NULL, NULL);
        expect("(");
        if (!consume(";")) {
            if (isTypeName()) {
                node->init = localVariable();
            } else {
                node->init = statementExpression();
                expect(";");
            }
        }
        if (!consume(";")) {
            node->cond = expression();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = statementExpression();
            expect(")");
        }
        node->body = statement();
    } else if (consume("{")) {
        Scope* currentScope = saveScope();
        node = newNode(NODE_BLOCK, NULL, NULL);
        Node dummy = {};
        Node* tail = &dummy;
        while (!consume("}")) {
            tail->next = statement();
            tail = tail->next;
        }
        node->statements = dummy.next;
        restoreScope(currentScope);
    } else {
        node = statementExpression();
        expect(";");
    }
    return node;
}

// structMember = basetype declarator typeSuffix ";"
static Member* structMember()
{
    Type* t = basetype(NULL);
    char* name = NULL;
    t = declarator(t, &name);
    t = typeSuffix(t);
    expect(";");

    Member* m = calloc(1, sizeof(Member));
    m->name = name;
    m->type = t;
    return m;
}

// structMembers = structMember* "}"
static Type* structMembers()
{
    Type* type = calloc(1, sizeof(Type));
    type->kind = TYPE_STRUCT;
    int align = 0;
    while (!consume("}")) {
        Member* m = structMember();
        if (type->members) {
            m->offset = alignOffset(type->members->offset, m->type->align) + size(m->type);
        } else {
            m->offset = size(m->type);
        }
        m->next = type->members;
        type->members = m;
        if (align < m->type->align) {
            align = m->type->align;
        }
    }
    type->align = align;
    return type;
}

// structDeclaration = "struct" identifier
//                   | "struct" identifier? "{" structMembers
static Type* structDeclaration()
{
    expect("struct");
    char* identifier = consumeIdentifier();
    if (identifier) {
        if (consume("{")) {
            // declare struct tag
            StructTag* tag = calloc(1, sizeof(StructTag));
            tag->name = identifier;
            tag->type = structMembers();
            tag->next = structTagScope;
            structTagScope = tag;
            return tag->type;
        } else {
            // find tag
            for (StructTag* s = structTagScope; s; s = s->next) {
                if (strlen(identifier) == strlen(s->name)
                    && !strcmp(identifier, s->name)) {
                    return s->type;
                }
            }
            // declare struct tag
            Type* type = calloc(1, sizeof(Type));
            type->kind = TYPE_STRUCT;
            type->align = 1;
            StructTag* tag = calloc(1, sizeof(StructTag));
            tag->name = identifier;
            tag->type = type;
            tag->next = structTagScope;
            structTagScope = tag;
            return type;
        }
    }
    // anonymous struct
    expect("{");
    return structMembers();
}

// enumDeclaration = "enum" identifier
//                 | "enum" identifier? "{" enumList* "}"
// enumList = identifier ("=" number)? ("," identifier ("=" number)?)* ","?
static Type* enumDeclaration()
{
    expect("enum");
    char* tag = consumeIdentifier();
    if (tag && !peek("{")) {
        return enumType();
    }
    expect("{");
    int enumVal = 0;
    while (!consume("}")) {
        char* name = expectIdentifier();
        if (consume("=")) {
            enumVal = expectNumber();
        }
        addEnumToVarScope(name, enumVal);
        ++enumVal;
        consume(",");
    }
    return enumType();
}

// basetype = (builtinType | structDeclaration | enumDeclaration | typedefName) "*"*
// builtInType = "void" | "char" | "short" | "int" | "long"
static Type* basetype(bool* isTypedef)
{
    if (isTypedef) {
        *isTypedef = false;
    }

    if (consume("typedef")) {
        if (!isTypedef) {
            errorMsg("typedef is not allowed here");
        }
        *isTypedef = true;
    }

    if (consume("void")) {
        return voidType();
    } else if (consume("char")) {
        return charType();
    } else if (consume("short")) {
        return shortType();
    } else if (consume("int")) {
        return intType();
    } else if (consume("long")) {
        return longType();
    } else if (peek("struct")) {
        return structDeclaration();
    } else if (peek("enum")) {
        return enumDeclaration();
    } else {
        Type* type = findTypedef(expectIdentifier());
        if (!type) {
            errorMsg("unexpected basetype");
        }
        return type;
    }
}

// parameter = basetype declarator typeSuffix
static Node* parameter()
{
    Type* type = basetype(NULL);
    char* name = NULL;
    type = declarator(type, &name);
    Variable* localVariable = declareLocalVariable(type, name);
    Node* node = newNode(NODE_NULL, NULL, NULL);
    node->type = type;
    return node;
}

// parameters = parameter ("," parameter)*
static Node* parameters()
{
    Node head = {};
    Node* tail = &head;
    tail->next = parameter();
    tail = tail->next;
    while (consume(",")) {
        tail->next = parameter();
        tail = tail->next;
    }
    return head.next;
}

// function = basetype declarator "(" parameters? ")" "{" statement* "}"
static Function* function()
{
    Type* retType = basetype(NULL);
    char* name;
    retType = declarator(retType, &name);
    expect("(");

    addFuncToVarScope(name, retType);
    localVariables = NULL;
    Scope* currentScope = saveScope();
    Function* func = calloc(1, sizeof(Function));
    func->name = name;
    if (!consume(")")) {
        func->params = parameters();
        expect(")");
    }
    expect("{");
    Node head;
    head.next = NULL;
    Node* tail = &head;
    while (!consume("}")) {
        tail->next = statement();
        tail = tail->next;
    }
    func->statements = head.next;
    func->localVariables = localVariables;
    if (localVariables) {
        func->stackSize = localVariables->variable->offset;
    } else {
        func->stackSize = 0;
    }
    addType(func->statements);
    restoreScope(currentScope);
    return func;
}

static Variable* declareGlobalVariable(Type* type, char* name)
{
    Variable* v = calloc(1, sizeof(Variable));
    v->name = name;
    v->type = type;
    v->isGlobal = true;
    VariableList* l = calloc(1, sizeof(VariableList));
    l->variable = v;
    l->next = globalVariables;
    globalVariables = l;
    addVarToVarScope(v);
    return v;
}

// globalVariableInitializer = number
static void globalVariableInitializer(Variable* v)
{
    v->initialValue = calloc(1, sizeof(InitialValue));
    char* str = consumeString();
    if (str) {
        if (v->type->kind == TYPE_POINTER
            && v->type->pointerTo->kind == TYPE_CHAR) {
            char* label = stringLabel();
            Type* type = arrayOf(charType(), strlen(str) + 1);
            declareGlobalVariable(type, label);
            globalVariables->variable->string = str;
            v->initialValue->label = label;
            return;
        } else if (v->type->kind == TYPE_ARRAY
            && v->type->arrayOf->kind == TYPE_CHAR) {
            v->initialValue->string = str;
            if (v->type->arraySize == 0) {
                v->type->arraySize = strlen(str) + 1;
            }
            return;
        }
        error("hoge");
        return;
    }
    if (consume("&")) {
        char* label = expectIdentifier();
        v->initialValue->label = label;
        return;
    }
    if (consume("{")) {
        int i = 0;
        ValueList dummy;
        ValueList* tail = &dummy;
        while (!consume("}")) {
            ValueList* val = calloc(1, sizeof(ValueList));
            val->value = expectNumber();
            tail->next = val;
            tail = val;
            consume(",");
            ++i;
        }
        if (v->type->arraySize == 0) {
            v->type->arraySize = i;
        }
        for (; i < v->type->arraySize; i++) {
            ValueList* val = calloc(1, sizeof(ValueList));
            val->value = 0;
            tail->next = val;
            tail = val;
        }
        v->initialValue->valueList = dummy.next;
        return;
    }
    ValueList* l = calloc(1, sizeof(ValueList));
    l->value = expectNumber();
    v->initialValue->valueList = l;
}

// declarator = "*"* ("(" declarator ")" | identifier) typeSuffix
static Type* declarator(Type* t, char** name)
{
    while (consume("*")) {
        t = pointerTo(t);
    }

    if (consume("(")) {
        Type* placeholder = calloc(1, sizeof(Type));
        Type* newType = declarator(placeholder, name);
        expect(")");
        memcpy(placeholder, typeSuffix(t), sizeof(Type));
        return newType;
    }

    *name = expectIdentifier();
    return typeSuffix(t);
}

// globalVariable = basetype declarator typeSuffix ("=" globalVariableInitializer)? ";"
static void globalVariable()
{
    bool isTypedef;
    Type* t = basetype(&isTypedef);
    char* name = NULL;
    t = declarator(t, &name);
    t = typeSuffix(t);
    if (isTypedef) {
        Typedef* tdef = calloc(1, sizeof(Typedef));
        tdef->name = name;
        tdef->type = t;
        tdef->next = typedefScope;
        typedefScope = tdef;
    } else {
        Variable* v = declareGlobalVariable(t, name);
        if (consume("=")) {
            globalVariableInitializer(v);
        }
    }
    expect(";");
}

bool isFunc()
{
    Token* token = rp;
    bool isTypedef;
    Type* type = basetype(&isTypedef);
    char* name = NULL;
    declarator(type, &name);
    if (consume("(")) {
        rp = token;
        return true;
    }
    rp = token;
    return false;
}

static Program* program()
{
    Function dummyFunc;
    Function* funcs = &dummyFunc;

    while (rp->kind != TOKEN_EOF) {
        if (isFunc()) {
            funcs->next = function();
            funcs = funcs->next;
        } else {
            globalVariable();
        }
    }

    Program* prog = calloc(1, sizeof(Program));
    prog->functions = dummyFunc.next;
    prog->globalVariables = globalVariables;
    return prog;
}

Program* parse(Token* tokens, char* source, char* fileName)
{
    rp = tokens;
    src = source;
    file = fileName;
    return program();
}
