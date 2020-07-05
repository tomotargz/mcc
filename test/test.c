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

int proxy(int num)
{
    return num;
}

int g1;
int g2 = 3;
char g3 = 3;
char* g4 = "string";
int* g5 = &g2;
int g6[3] = { 1, 2, 3 };
int g7[3] = { 1 };
int g8[] = { 1, 2, 3 };
char g9[] = "string";
char g10[7] = "string";
struct gs {
    int a;
};

static int sg1;
static int sg = 3;

int* returnPointer()
{
    return &g2;
}

char* pointerToStr()
{
    return "string";
}

void voidFunc()
{
    return;
}

int declareFunction(int param);

static int staticFunc(int param);
static int staticFunc(int param)
{
    return 3;
}

/* block comment */
// line comment

typedef int gmyint1;
typedef int gmyint2;

int main()
{
    {
        typedef int myint1;
        typedef int myint2;
    }

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

    assertEq(3, ({g1 = 3;g1; }), "g1 = 3;g1;");
    assertEq(5, ({int ga;ga = 5;ga; }), "ga = 5;ga;");

    assertEq(3, ({int a = 3;a; }), "int a = 3;a;");
    {
        int a = 0;
        for (int i = 0; i <= 3; i = i + 1) {
            a = i;
        }
        assertEq(3, a, "");
    }

    assertEq(105, ({char* a = "string"; a[3]; }), "char* a = \"string\"; a[3]; }");
    assertEq(105, ({char a[] = "string"; a[3]; }), "char a[] = \"string\"; a[3]; }");
    assertEq(7, ({char a[] = "string"; sizeof(a); }), "char a[] = \"string\"; sizeof(a); }");
    assertEq(3, ({int a[3] = {1,2,3}; a[2]; }), "int a[3] = {1,2,3}; a[2];");
    assertEq(0, ({int a[5] = {}; a[3]; }), "int a[5] = {}; a[3];");
    assertEq(3, ({int a[] = {1,2,3}; a[2]; }), "int a[] = {1,2,3}; a[2];");
    assertEq(12, ({int a[] = {1,2,3}; sizeof(a); }), "int a[] = {1,2,3}; sizeof(a);");

    assertEq(3, ({ g2; }), "g2");
    assertEq(3, ({ g3; }), "g3");
    assertEq(105, ({ *(g4 + 3); }), "*(g4 + 3);");
    assertEq(105, ({ g4[3]; }), "g4[3];");
    assertEq(3, ({ *g5; }), "*g5;");

    assertEq(3, ({ g6[2]; }), "g6[2];");
    assertEq(0, ({ g7[2]; }), "g7[2];");
    assertEq(3, ({ g8[2]; }), "g8[3];");
    assertEq(12, ({ sizeof(g8); }), "sizeof(g8);");
    assertEq(105, ({ g9[3]; }), "g9[3];");
    assertEq(7, ({ sizeof(g9); }), "sizeof(g9);");
    assertEq(105, ({ g10[3]; }), "g10[3];");
    assertEq(7, ({ sizeof(g10); }), "sizeof(g10);");

    assertEq(3, ({struct {int a;int b;}s; 3; }), "struct {int a;int b;}s; 3;");
    assertEq(3, ({struct {int a;int b;}s; s.a = 3; s.a; }), "struct {int a;int b;}s; s.a = 3; s.a;");
    assertEq(3, ({struct {int a;int b;} s[4]; s[3].a = 3; s[3].a; }), "struct {int a;int b;} s[4]; s[3].a = 3; s[3].a;");
    assertEq(8, ({struct {int a;int b;} s; sizeof(s); }), "struct {int a;int b;} s; sizeof(s);");
    assertEq(24, ({struct {int a;int b;} s[3]; sizeof(s); }), "struct {int a;int b;} s[3]; sizeof(s);");
    assertEq(3, ({struct {struct{int a; int b;}internal; int c;} s; s.internal.a = 3;s.internal.a; }), "struct {struct{int a; int b;}internal; int c;} s; s.internal.a = 3;s.internal.a;");
    assertEq(12, ({struct {char a; int b; char c;}s; sizeof(s); }), "{struct {char a; int b; char c;}s; sizeof(s);");
    assertEq(3, ({struct tag{int a; int b;}; struct tag s; s.a = 3; s.a; }), "struct tag{int a; int b;}; struct tag s; s.a = 3; s.a;");
    assertEq(3, ({struct tag{int a; int b;} s; s.a = 3; s.a; }), "struct tag{int a; int b;} s; s.a = 3; s.a;");
    assertEq(8, ({struct tag{int a; int b;}; struct tag s; sizeof(s); }), "struct tag{int a; int b;}; struct tag s; sizeof(s);");
    assertEq(3, ({struct tag{int a; int b;} s; struct tag* sp = &s; sp->a = 3; sp->a; }), "struct tag{int a; int b;} s; struct tag* sp = &s; sp->a = 3; sp->a;}");

    assertEq(3, ({char a; int b; char c; b = 3; b; }), "char a; int b; char c; b = 3; b;");

    assertEq(3, ({typedef struct Node{int num;}Node; Node n; n.num = 3; n.num; }), "typedef struct Node{int num;}Node; Node n; n.num = 3; n.num;");
    assertEq(3, ({typedef int MyArray[5]; MyArray a; a[3] = 3;a[3]; }), "typedef int MyArray[5]; MyArray a; a[3] = 3;a[3];");

    assertEq(1, ({enum {A,B,C};B; }), "enum {A,B,C};B;}");
    assertEq(3, ({enum {A = 1,B,C};C; }), "enum {A = 1,B,C};C;}");
    assertEq(4, ({enum {A,B,C} e;sizeof(e); }), "enum {A,B,C} e;sizeof(e);");
    assertEq(1, ({enum tag {A,B,C};enum tag e; e = B;e; }), "enum tag{A,B,C};enum tag e; e = B;e;");
    assertEq(1, ({typedef enum {A,B,C} E;E e;e = B;e; }), "typedef enum {A,B,C} E;E e;e = B;e;");

    assertEq(1, ({ sizeof(char); }), "sizeof(char);");
    assertEq(4, ({ sizeof(int); }), "sizeof(int);");
    assertEq(8, ({ sizeof(int*); }), "sizeof(int*);");
    assertEq(20, ({ sizeof(int[5]); }), "sizeof(int[5]);");
    assertEq(8, ({ struct s{int a; char b;};sizeof(struct s); }), "struct s{int a; char b;};sizeof(struct s);");
    assertEq(8, ({ typedef struct s{int a; char b;} s;sizeof(s); }), "typedef struct s{int a; char b;} s;sizeof(s);");
    assertEq(8, ({ sizeof(int(*)[5]); }), "sizeof(int(*)[5]);");
    assertEq(8, ({ sizeof(int*(*)[5]); }), "sizeof(int(*)[5]);");
    assertEq(40, ({ sizeof(int(*[5])[5]); }), "sizeof(int(*[5])[5]);");

    assertEq(2, ({int i = 1; ++i; i; }), "int i = 1; ++i; i; ");
    assertEq(2, ({int i = 1; i++; i; }), "int i = 1; i++; i; ");
    assertEq(0, ({int i = 1; --i; i; }), "int i = 1; --i; i; ");
    assertEq(0, ({int i = 1; i--; i; }), "int i = 1; i--; i; ");
    assertEq(2, ({int i = 1; proxy(++i); }), "int i = 1; proxy(++i);");
    assertEq(1, ({int i = 1; proxy(i++); }), "int i = 1; proxy(i++);");
    assertEq(2, ({int i = 1; proxy(i++);i; }), "int i = 1; proxy(i++); i;");
    assertEq(0, ({int i = 1; proxy(--i); }), "int i = 1; proxy(--i);");
    assertEq(1, ({int i = 1; proxy(i--); }), "int i = 1; proxy(i--);");
    assertEq(0, ({int i = 1; proxy(i--);i; }), "int i = 1; proxy(i--); i;");
    assertEq(3, ({int a[3];a[1] = 3;int* p = a;p++;*p; }), "int a[3];a[1] = 3;int* p = a;p++;*p;");
    assertEq(3, ({int a[3];a[1] = 3;int* p = a;++p;*p; }), "int a[3];a[1] = 3;int* p = a;++p;*p;");
    assertEq(3, ({int a[3];a[1] = 3;int* p = a + 2;p--;*p; }), "int a[3];a[1] = 3;int* p = a + 2;p--;*p;");
    assertEq(3, ({int a[3];a[1] = 3;int* p = a + 2;--p;*p; }), "int a[3];a[1] = 3;int* p = a + 2;--p;*p;");

    assertEq(3, ({ *returnPointer(); }), "*returnPointer();");
    assertEq(105, ({ *(pointerToStr() + 3); }), "*(pointerToStr() + 3);");

    assertEq(0, ({ !1; }), "!1");
    assertEq(1, ({ !0; }), "!0");
    assertEq(1, ({ 1 && 1; }), "1 && 1;");
    assertEq(0, ({ 1 && 0; }), "1 && 0;");
    assertEq(0, ({ 0 && 1; }), "0 && 1;");
    assertEq(0, ({ 0 && 0; }), "0 && 0;");
    assertEq(1, ({ 1 || 1; }), "1 || 1;");
    assertEq(1, ({ 1 || 0; }), "1 || 0;");
    assertEq(1, ({ 0 || 1; }), "0 || 1;");
    assertEq(0, ({ 0 || 0; }), "0 || 0;");

    assertEq(3, ({short s = 3; s; }), "short s = 3; s;");
    assertEq(2, ({ sizeof(short); }), " sizeof(short);");
    assertEq(3, ({long l = 3; l; }), "long l = 3; l;");
    assertEq(8, ({ sizeof(long); }), " sizeof(long);");

    assertEq(3, ({ staticFunc(3); }), " staticFunc(3); ");

    {
        void* v;
    }

    voidFunc();

    gmyint1 gmyint = 1;

    printf("OK\n");
    return 0;
}
