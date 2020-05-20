#include "type.h"
#include "error.h"
#include "stdlib.h"

Type* pointerTo(Type* type)
{
    Type* newType = calloc(1, sizeof(Type));
    newType->type = TYPE_POINTER;
    newType->pointTo = type;
    return newType;
}

TypeKind pointee(Type* type)
{
    if (!type || type->type != TYPE_POINTER) {
        error("given type is not a pointer");
    }
    return type->pointTo->type;
}
