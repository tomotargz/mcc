#ifndef TYPE_H
#define TYPE_H

typedef enum TypeKind {
    INT,
    PTR
} TypeKind;

typedef struct Type {
    TypeKind type;
    struct Type* pointTo;
} Type;

#endif
