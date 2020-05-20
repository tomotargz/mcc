#ifndef TYPE_H
#define TYPE_H

typedef enum TypeKind {
    TYPE_INT,
    TYPE_POINTER,
    TYPE_NO
} TypeKind;

typedef struct Type {
    TypeKind type;
    struct Type* pointTo;
} Type;

Type* pointerTo(Type* type);
TypeKind pointee(Type* type);

#endif
