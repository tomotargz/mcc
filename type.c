#include "type.h"
#include "error.h"
#include "stdlib.h"

Type* newPointer(Type* to)
{
    Type* newType = calloc(1, sizeof(Type));
    newType->kind = TYPE_POINTER;
    newType->pointerTo = to;
    return newType;
}
