int assertEq(int expected, int actual)
{
    if (expected != actual) {
        printf("test failed!! expected:%d actual:%d", expected, actual);
    }
    return 0;
}

int main()
{
    assertEq(1,({int aaaa;aaaa = 2;aaaa;}));
    return 3;
}
