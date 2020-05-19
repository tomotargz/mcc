#!/bin/bash

cat <<EOF | gcc -xc -c -o tmp2.o -
int func1() { return 3; }
int func2(int a, int b, int c, int d, int e, int f) { return a+b+c+d+e+f; }
EOF

try() {
    expected="$1"
    input="$2"

    ./mcc "$input" > tmp.s
    gcc -O0 -o tmp tmp.s tmp2.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expectd, but got $actual"
        exit 1
    fi
}

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
