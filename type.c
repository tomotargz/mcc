#include "mcc.h"

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
    } else if (type->kind == TYPE_INT
        || type->kind == TYPE_ENUM) {
        return 4;
    } else if (type->kind == TYPE_POINTER) {
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
