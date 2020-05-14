struct Type {
    enum {
        INT,
        PTR
    } ty;
    struct Type* ptr_to;
};
