#include "type.h"
#include "error.h"
#include "stdlib.h"

Type* pointerTo(Type* type)
{
    Type* newType = calloc(1, sizeof(Type));
    newType->kind = TYPE_POINTER;
    newType->pointTo = type;
    return newType;
}

TypeKind pointee(Type* type)
{
    if (!type || type->kind != TYPE_POINTER) {
        error("given type is not a pointer");
    }
    return type->pointTo->kind;
}
