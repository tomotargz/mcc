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

int multiReturn()
{
    return 3;
    return 0;
}

int funcWith6Args(int a, int b, int c, int d, int e, int f)
{
    return a + b + c + d + e + f;
}

int ga;

/* block comment */
// line comment

int main()
{
    assertEq(3, ({int a[10];a[3] = 3;*(a+3); }), "int a[10];a[3] = 3;*(a+3);");
    assertEq(3, ({int a[10];a[1] = 3;*(a+3-2); }), "int a[10];a[1] = 3;*(a+3-2);");
    assertEq(3, ({int* a[10];int b[10];b[3] = 3;a[3] = b;*(*(a+3)+3); }), "int* a[10];int b[10];b[3] = 3;a[3] = b;*(*(a+3)+3);");

    assertEq(3, ({int a;a = 3;a; }), "int a;a = 3;a;");
    assertEq(3, ({int a;int b;a = 3;b = a; b; }), "int a;int b;a = 3;b = a; b;");

    assertEq(3, ({char a;a = 3;a; }), "char a;a = 3;a;");
    assertEq(3, ({char a[10]; a[3]=3; a[3]; }), "char a[10]; a[3]=3; a[3];");

    assertEq(105, ({ "string"[3]; }), "\"string\"[3];");
    assertEq(105, ({char* s;s = "string";s[3]; }), "char* s;s = \"string\";s[3];");

    assertEq(3, ({ 3; }), "3;");
    assertEq(6, ({ 1 + 2 + 3; }), "1+2+3;");
    assertEq(0, ({ 3 - 2 - 1; }), "3-2-1;");
    assertEq(6, ({ 1 * 2 * 3; }), "1*2*3;");
    assertEq(1, ({ 6 / 3 / 2; }), "6/3/2;");
    assertEq(1, ({ +1; }), "+1;");
    assertEq(2, ({ -1 + 3; }), "-1+3;");
    assertEq(5, ({ (-1 + 2) * 3 + 6 / 3; }), "(-1+2)*3+6/3;");
    assertEq(0, ({ 1 == 0; }), "1==0;");
    assertEq(1, ({ 1 == 1; }), "1==1;");
    assertEq(1, ({ 1 != 0; }), "1!=0;");
    assertEq(0, ({ 1 != 1; }), "1!=1;");
    assertEq(1, ({ 0 < 1; }), "0<1;");
    assertEq(0, ({ 1 < 0; }), "1<0;");
    assertEq(1, ({ 0 <= 0; }), "0<=0;");
    assertEq(1, ({ 0 <= 1; }), "0<=1;");
    assertEq(0, ({ 1 <= 0; }), "1<=0;");
    assertEq(0, ({ 0 > 1; }), "0>1;");
    assertEq(1, ({ 1 > 0; }), "1>0;");
    assertEq(1, ({ 0 >= 0; }), "0>=0;");
    assertEq(0, ({ 0 >= 1; }), "0>=1;");
    assertEq(1, ({ 1 >= 0; }), "1>=0;");

    assertEq(3, ({int longName; longName = 3; longName; }), "int longName; longName = 3; longName;");

    assertEq(3, ({ multiReturn(); }), "multiReturn()");

    {
        int v;
        if (1 == 1)
            v = 1;
        else
            v = 0;
        assertEq(1, v, "int v;if(1==1) v = 1;else v = 0;");
    }

    {
        int v;
        if (1 == 0)
            v = 0;
        else
            v = 1;
        assertEq(1, v, "int v;if(1==0) v = 0;else v = 1;");
    }

    {
        int a;
        a = 0;
        while (a != 3) {
            a = a + 1;
        }
        assertEq(3, a, "int a;a = 0;while(a != 3){a = a + 1;}");
    }

    {
        int a;
        for (a = 0; a < 3; a = a + 1) {
        }
        assertEq(3, a, "int a;for(a = 0; a < 3; a = a + 1){}");
    }

    assertEq(21, ({ funcWith6Args(1, 2, 3, 4, 5, 6); }), "funcWith6Args(1,2,3,4,5,6)");

    assertEq(4, ({int a;sizeof(a); }), "int a;sizeof(a);");
    assertEq(1, ({char a;sizeof(a); }), "char a;sizeof(a);");
    assertEq(8, ({int* a;sizeof(a); }), "int* a;sizeof(a);");
    assertEq(40, ({int a[10];sizeof(a); }), "int a[10];sizeof(a);");

    assertEq(3, ({int a;int* b;b = &a;*b = 3;a; }), "int a;int* b;b = &a;*b = 3;a;");

    assertEq(3, ({int a[10];*a=3;*a; }), "int a[10];*a=3;*a;");
    assertEq(3, ({int a[10];*(a+1)=3;*(a+1); }), "int a[10];*(a+1)=3;*(a+1);");
    assertEq(3, ({int a[10];a[3]=3;a[3]; }), "int a[10];a[3]=3;a[3];");

    assertEq(3, ({ga = 3;ga; }), "ga = 3;ga;");
    assertEq(5, ({int ga;ga = 5;ga; }), "ga = 5;ga;");

    assertEq(3, ({int a = 3;a; }), "int a = 3;a;");
    {
        int a = 0;
        for (int i = 0; i <= 3; i = i + 1) {
            a = i;
        }
        assertEq(3, a, "");
    }

    printf("OK\n");
    return 0;
}
