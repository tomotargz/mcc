#ifndef TYPE_H
#define TYPE_H

typedef enum TypeKind {
    TYPE_INT,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_NO
} TypeKind;

typedef struct Type {
    TypeKind kind;
    struct Type* pointTo;
    int arraySize;
} Type;

Type* pointerTo(Type* type);
TypeKind pointee(Type* type);

#endif
