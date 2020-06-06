int assertEq(int expected, int actual)
{
    if (expected != actual) {
        printf("test failed!! expected:%d actual:%d", expected, actual);
    }
    return 0;
}

int main()
{
    assertEq(1, ({int a;a = 1;a; }));
    return 3;
}
