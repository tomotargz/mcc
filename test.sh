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

try 3 "main(){a=3;b=&a;return *b;}"
try 3 "add(a){r = a;return r;}main(){return add(3);}"
try 21 "add(a,b,c,d,e,f){ret = a+b+c+d+e+f;return ret;}main(){return add(1,2,3,4,5,6);}"
try 6 "func(){return 1+2+3;}main(){return func();}"
try 1 "main(){a=1;return a;}"
try 3 "main(){func1();}"
try 21 "main(){return func2(1,2,3,4,5,6);}"
try 0 "main(){return 0;}"
try 6 "main(){return 1+2+3;}"
try 0 "main(){return 3-2-1;}"
try 6 "main(){return 1*2*3;}"
try 1 "main(){return 6/3/2;}"
try 1 "main(){return +1;}"
try 2 "main(){return -1+3;}"
try 5 "main(){return (-1+2)*3+6/3;}"
try 0 "main(){return 1==0;}"
try 1 "main(){return 1==1;}"
try 1 "main(){return 1!=0;}"
try 0 "main(){return 1!=1;}"
try 1 "main(){return 0<1;}"
try 0 "main(){return 1<0;}"
try 1 "main(){return 0<=0;}"
try 1 "main(){return 0<=1;}"
try 0 "main(){return 1<=0;}"
try 0 "main(){return 0>1;}"
try 1 "main(){return 1>0;}"
try 1 "main(){return 0>=0;}"
try 0 "main(){return 0>=1;}"
try 1 "main(){return 1>=0;}"
try 1 "main(){a=1;return a;}"
try 1 "main(){a=1;b=a;return b;}"
try 6 "main(){hoge=1;fuga=hoge*2;piyo=fuga*3;return piyo;}"
try 1 "main(){return 1;return 2;}"
try 1 "main(){if(1==1) return 1; return 0;}"
try 0 "main(){if(0==1) return 1; return 0;}"
try 1 "main(){if(1==1) return 1; else return 0;}"
try 0 "main(){if(0==1) return 1; else return 0;}"
try 1 "main(){a=0;while(a==0)return 1;return 0;}"
try 0 "main(){a=0;while(a==1)return 1;return 0;}"
try 3 "main(){a=0;while(a<3)a=a+1;return a;}"
try 3 "main(){for(i=0;i<3;i=i+1)0;return i;}"
try 3 "main(){for(i=0;i<3;)i=i+1;return i;}"
try 3 "main(){for(i=0;;i=i+1)if(i>=3)return i;}"
try 3 "main(){i=0;for(;i<3;i=i+1)0;return i;}"
try 0 "main(){{a=1;b=2;c=3;return 0;}}"
try 1 "main(){if(1==1){a=1;return a;}return 0;}"
try 6 "main(){i=0;while(i<5){i=i+1;i=i+1;}return i;}"
try 2 "main(){i=0;{i=1;i=2;}return i;}"
try 7 "main(){{1+2;2+3;return 3+4;}}"


echo OK
