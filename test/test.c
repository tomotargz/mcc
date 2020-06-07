int assertEq(int expected, int actual)
{
    if (expected != actual) {
        printf("test failed!! expected:%d actual:%d", expected, actual);
    }else{
        printf("succeeded");
    }
    return 0;
}

int main()
{
    assertEq(2, ({int a;a = 2;a; }));
    return 3;
}
