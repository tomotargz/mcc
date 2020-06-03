#include "type.h"
#include "error.h"
#include "stdlib.h"

Type* pointerTo(Type* type)
{
    Type* newType = calloc(1, sizeof(Type));
    newType->kind = TYPE_POINTER;
    newType->pointerTo = type;
    return newType;
}

Type* arrayOf(Type* type, int size)
{
    Type* array = calloc(1, sizeof(Type));
    array->kind = TYPE_ARRAY;
    array->arrayOf = type;
    array->arraySize = size;
    return array;
}

int size(Type* type)
{
    if (type->kind == TYPE_INT) {
        return 4;
    } else if (type->kind == TYPE_POINTER) {
        return 8;
    } else if (type->kind == TYPE_ARRAY) {
        return size(type->arrayOf) * type->arraySize;
    } else if (type->kind == TYPE_CHAR) {
        return 1;
    }
    error("invalid type");
    return 0;
}
