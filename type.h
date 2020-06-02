#ifndef TYPE_H
#define TYPE_H

#include <stddef.h>

typedef enum TypeKind {
    TYPE_INT,
    TYPE_CHAR,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_NO
} TypeKind;

typedef struct Type {
    TypeKind kind;
    struct Type* pointerTo;
    struct Type* arrayOf;
    int arraySize;
} Type;

static Type INT_TYPE = {TYPE_INT, NULL, NULL, 0};
static Type CHAR_TYPE = {TYPE_CHAR, NULL, NULL, 0};

Type* pointerTo(Type* to);
Type* arrayOf(Type* type, int size);

#endif
