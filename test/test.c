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

int main()
{
    assertEq(3, ({int a;a = 3;a; }), "({int a;a = 3;a; })");
    printf("OK\n");
    return 0;
}
