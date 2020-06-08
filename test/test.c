int assertEq(int expected, int actual, char* code)
{
    if (expected != actual) {
        printf("Test failed\n");
        printf("code: %s\n", code);
        printf("expected: %d\n", expected);
        printf("actual: %d\n", actual);
        exit(1);
    }
    return 0;
}

/* block comment */

int main()
{
    assertEq(3, ({int a;a = 3;a; }), "int a;a = 3;a;");
    assertEq(105, ({ "string"[3]; }), "\"string\"[3];");
    assertEq(105, ({char* s;s = "string";s[3]; }), "char* s;s = \"string\";s[3];");
    assertEq(3, ({char a;char b;a = 1;b = 2;a + b; }), "char a;char b; a = 1; b = 2; a + b;");
    printf("OK\n");
    return 0;
}
