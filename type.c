#include "mcc.h"

Type* voidType()
{
    Type* t = calloc(1, sizeof(Type));
    t->kind = TYPE_VOID;
    t->align = 1;
    return t;
}

Type* charType()
{
    Type* t = calloc(1, sizeof(Type));
    t->kind = TYPE_CHAR;
    t->align = 1;
    return t;
}

Type* shortType()
{
    Type* t = calloc(1, sizeof(Type));
    t->kind = TYPE_SHORT;
    t->align = 2;
    return t;
}

Type* intType()
{
    Type* t = calloc(1, sizeof(Type));
    t->kind = TYPE_INT;
    t->align = 4;
    return t;
}

Type* longType()
{
    Type* t = calloc(1, sizeof(Type));
    t->kind = TYPE_LONG;
    t->align = 8;
    return t;
}

Type* enumType()
{
    Type* t = calloc(1, sizeof(Type));
    t->kind = TYPE_ENUM;
    t->align = 4;
    return t;
}

Type* noType()
{
    Type* t = calloc(1, sizeof(Type));
    t->kind = TYPE_NO;
    t->align = 0;
    return t;
}

Type* pointerTo(Type* type)
{
    Type* pointer = calloc(1, sizeof(Type));
    pointer->kind = TYPE_POINTER;
    pointer->align = 4;
    pointer->pointerTo = type;
    return pointer;
}

Type* arrayOf(Type* type, int size)
{
    Type* array = calloc(1, sizeof(Type));
    array->kind = TYPE_ARRAY;
    array->align = type->align;
    array->arrayOf = type;
    array->arraySize = size;
    return array;
}

int size(Type* type)
{
    if (type->kind == TYPE_CHAR) {
        return 1;
    } else if (type->kind == TYPE_SHORT) {
        return 2;
    } else if (type->kind == TYPE_INT
        || type->kind == TYPE_ENUM) {
        return 4;
    } else if (type->kind == TYPE_LONG
        || type->kind == TYPE_POINTER) {
        return 8;
    } else if (type->kind == TYPE_ARRAY) {
        return size(type->arrayOf) * type->arraySize;
    } else if (type->kind == TYPE_STRUCT) {
        return alignOffset(type->members->offset, type->align);
    }
    error("invalid type");
    return 0;
}

int alignOffset(int offset, int align)
{
    if (align <= 0) {
        error("invalid align");
    }
    int diff = offset;
    while (diff >= align) {
        diff = diff - align;
    }
    if (diff) {
        return offset + align - diff;
    }
    return offset;
}

bool isInteger(Type* type)
{
    return type->kind == TYPE_CHAR
        || type->kind == TYPE_SHORT
        || type->kind == TYPE_INT
        || type->kind == TYPE_LONG;
}

bool isPointer(Type* type)
{
    return type->kind == TYPE_POINTER;
}

bool isPointerOrArray(Type* type)
{
    return type->kind == TYPE_POINTER
        || type->kind == TYPE_ARRAY;
}
