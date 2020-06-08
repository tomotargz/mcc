#!/bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
#include <stdio.h>
int func1() { return 3; }
int func2(int a, int b, int c, int d, int e, int f) { return a+b+c+d+e+f; }
int* array(){static int a[] = {0,1,2,3,4};return a;}
int** array2d(){static int a[] = {0,1};static int b[] = {1,2};static int c[] = {2,3};static int* d[] = {a,b,c};return d;}
void myPrintf(char* str){printf("%s\n",str);}
EOF

test() {
    input="$1"

    ./mcc "$input" > tmp.s
    gcc -static -O0 -o tmp tmp.s tmp2.o
    ./tmp
}

test "test/test.c"
exit 0

try 3 "int main(){char a;char b;a=1;b=2;return (a+b);}"
try 3 "int main(){char c[2]; c[0] = -1;c[1] = 4;return c[0] + c[1];}"
try 3 "int main(){char c[2]; c[0] = 1;c[1] = 2;return c[0] + c[1];}"
try 2 "int main(){int* a;a = array();return *(a+2);}"
try 1 "int main(){int* a;a = array();return *(a+2-1);}"
try 2 "int main(){int** a;a = array2d();return *(*(a+2));}"
try 1 "int main(){int** a;a = array2d();return *(*(a+2-1));}"
try 3 "int main(){char a;a=3;return a;}"
try 3 "int main(){char a[10];a[3]=3;return a[3];}"
try 3 "int a;int assignToGlobal(int num){a = num;}int main(){int a;a=3;assignToGlobal(5);return a;}"
try 3 "int a[10];int main(){a[1] = 3; return a[1];}"
try 3 "int a;int main(){a = 3; return a;}"
try 3 "int ga; int* gb; int gc[3];int main(){int a; a = 3; return a;}"
try 3 "int main(){int y0[10];int y1[10];int* x[2];x[0] = y0;x[1] = y1;x[1][3] = 3;return x[1][3];}"
try 3 "int main(){int a[10];a[1] = 3;return a[1];}"
try 3 "int main(){int a[10];*a = 1;*(a+1) = 2;return *a + *(a+1);}"
try 3 "int main(){int a[10];*(a+1) = 3; return *(a+1);}"
try 3 "int main(){int a[10];*a = 3;return *a;}"
try 40 "int main(){int a[10]; return sizeof(a);}"
try 8 "int main(){int* a;return sizeof(a);}"
try 4 "int main(){int a;return sizeof(a);}"
try 3 "int main(){int a;int* b;b = &a;*b = 3;return a;}"
try 3 "int add(int a){int r;r = a;return r;}int main(){return add(3);}"
try 21 "int add(int a,int b,int c,int d,int e,int f){int ret;ret = a+b+c+d+e+f;return ret;}int main(){return add(1,2,3,4,5,6);}"
try 6 "int func(){return 1+2+3;}int main(){return func();}"
try 1 "int main(){int a;a=1;return a;}"
try 3 "int main(){func1();}"
try 21 "int main(){return func2(1,2,3,4,5,6);}"
try 0 "int main(){return 0;}"
try 6 "int main(){return 1+2+3;}"
try 0 "int main(){return 3-2-1;}"
try 6 "int main(){return 1*2*3;}"
try 1 "int main(){return 6/3/2;}"
try 1 "int main(){return +1;}"
try 2 "int main(){return -1+3;}"
try 5 "int main(){return (-1+2)*3+6/3;}"
try 0 "int main(){return 1==0;}"
try 1 "int main(){return 1==1;}"
try 1 "int main(){return 1!=0;}"
try 0 "int main(){return 1!=1;}"
try 1 "int main(){return 0<1;}"
try 0 "int main(){return 1<0;}"
try 1 "int main(){return 0<=0;}"
try 1 "int main(){return 0<=1;}"
try 0 "int main(){return 1<=0;}"
try 0 "int main(){return 0>1;}"
try 1 "int main(){return 1>0;}"
try 1 "int main(){return 0>=0;}"
try 0 "int main(){return 0>=1;}"
try 1 "int main(){return 1>=0;}"
try 1 "int main(){int a;a=1;return a;}"
try 1 "int main(){int a;int b;a=1;b=a;return b;}"
try 6 "int main(){int hoge;int fuga;int piyo;hoge=1;fuga=hoge*2;piyo=fuga*3;return piyo;}"
try 1 "int main(){return 1;return 2;}"
try 1 "int main(){if(1==1) return 1; return 0;}"
try 0 "int main(){if(0==1) return 1; return 0;}"
try 1 "int main(){if(1==1) return 1; else return 0;}"
try 0 "int main(){if(0==1) return 1; else return 0;}"
try 1 "int main(){int a;a=0;while(a==0)return 1;return 0;}"
try 0 "int main(){int a;a=0;while(a==1)return 1;return 0;}"
try 3 "int main(){int a;a=0;while(a<3)a=a+1;return a;}"
try 3 "int main(){int i;for(i=0;i<3;i=i+1)0;return i;}"
try 3 "int main(){int i;for(i=0;i<3;)i=i+1;return i;}"
try 3 "int main(){int i;for(i=0;;i=i+1)if(i>=3)return i;}"
try 3 "int main(){int i;i=0;for(;i<3;i=i+1)0;return i;}"
try 0 "int main(){{int a;int b;int c;a=1;b=2;c=3;return 0;}}"
try 1 "int main(){if(1==1){int a;a=1;return a;}return 0;}"
try 6 "int main(){int i;i=0;while(i<5){i=i+1;i=i+1;}return i;}"
try 2 "int main(){int i;i=0;{i=1;i=2;}return i;}"
try 7 "int main(){{1+2;2+3;return 3+4;}}"

echo OK
