#include "mcc.h"

static Type VOID_TYPE = { TYPE_VOID, 1, NULL, NULL, 0, NULL };
static Type CHAR_TYPE = { TYPE_CHAR, 1, NULL, NULL, 0, NULL };
static Type SHORT_TYPE = { TYPE_SHORT, 2, NULL, NULL, 0, NULL };
static Type INT_TYPE = { TYPE_INT, 4, NULL, NULL, 0, NULL };
static Type LONG_TYPE = { TYPE_LONG, 8, NULL, NULL, 0, NULL };
static Type ENUM_TYPE = { TYPE_ENUM, 4, NULL, NULL, 0, NULL };
static Type NO_TYPE = { TYPE_NO, 0, NULL, NULL, 0, NULL };

Type* voidType()
{
    return &VOID_TYPE;
}

Type* charType()
{
    return &CHAR_TYPE;
}

Type* shortType()
{
    return &SHORT_TYPE;
}

Type* intType()
{
    return &INT_TYPE;
}

Type* longType()
{
    return &LONG_TYPE;
}

Type* enumType()
{
    return &ENUM_TYPE;
}

Type* noType()
{
    return &NO_TYPE;
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
    int diff = offset % align;
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
