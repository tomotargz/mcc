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
