#ifndef TYPE_H
#define TYPE_H

#include <stddef.h>

typedef enum TypeKind {
    TYPE_INT,
    TYPE_CHAR,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_ENUM,
    TYPE_NO
} TypeKind;

typedef struct Type Type;

typedef struct Member {
    Type* type;
    char* name;
    int offset;
    struct Member* next;
} Member;

typedef struct Type {
    TypeKind kind;
    int align;
    // pointer
    struct Type* pointerTo;
    // array
    struct Type* arrayOf;
    int arraySize;
    // struct
    Member* members;
} Type;

static Type INT_TYPE = { TYPE_INT, 1, NULL, NULL, 0, NULL };
static Type CHAR_TYPE = { TYPE_CHAR, 4, NULL, NULL, 0, NULL };
static Type ENUM_TYPE = { TYPE_ENUM, 4, NULL, NULL, 0, NULL };
static Type NO_TYPE = { TYPE_NO, 0, NULL, NULL, 0, NULL };

Type* pointerTo(Type* to);
Type* arrayOf(Type* type, int size);
int size(Type* type);
int alignOffset(int offset, int align);

#endif
