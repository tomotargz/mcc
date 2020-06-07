int assertEq(int expected, int actual)
{
    if (expected != actual) {
        printf("Failed\n");
        printf("expected: %d\n", expected);
        exit(1);
    }
    return 0;
}

int main()
{
    assertEq(2, ({int a;a = 3;a; }));
    printf("OK\n");
    return 0;
}
