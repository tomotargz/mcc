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
    struct Type* pointerTo;
    struct Type* arrayOf;
    int arraySize;
} Type;

Type* pointerTo(Type* to);
Type* arrayOf(Type* type, int size);

#endif
