#include "type.h"
#include "stdlib.h"

Type* pointerTo(Type* type)
{
    Type* newType = calloc(1, sizeof(Type));
    newType->type = TYPE_POINTER;
    newType->pointTo = type;
    return newType;
}
