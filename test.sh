#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./mcc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expectd, but got $actual"
        exit 1
    fi
}

try 0 "0;"
try 6 "1+2+3;"
try 0 "3-2-1;"
try 6 "1*2*3;"
try 1 "6/3/2;"
try 1 "+1;"
try 2 "-1+3;"
try 5 "(-1+2)*3+6/3;"
try 0 "1==0;"
try 1 "1==1;"
try 1 "1!=0;"
try 0 "1!=1;"
try 1 "0<1;"
try 0 "1<0;"
try 1 "0<=0;"
try 1 "0<=1;"
try 0 "1<=0;"
try 0 "0>1;"
try 1 "1>0;"
try 1 "0>=0;"
try 0 "0>=1;"
try 1 "1>=0;"
try 1 "a=1;a;"
try 1 "a=1;b=a;b;"
try 6 "hoge=1;fuga=hoge*2;piyo=fuga*3;piyo;"
try 1 "return 1;return 2;"
try 1 "if(1==1) return 1; return 0;"
try 0 "if(0==1) return 1; return 0;"
try 1 "if(1==1) return 1; else return 0;"
try 0 "if(0==1) return 1; else return 0;"
try 1 "a=0;while(a==0)return 1;return 0;"
try 0 "a=0;while(a==1)return 1;return 0;"
try 3 "a=0;while(a<3)a=a+1;return a;"
try 3 "for(i=0;i<3;i=i+1)0;return i;"
try 3 "for(i=0;i<3;)i=i+1;return i;"
try 3 "for(i=0;;i=i+1)if(i>=3)return i;"
try 3 "i=0;for(;i<3;i=i+1)0;return i;"
try 0 "{a=1;b=2;c=3;return 0;}"
try 1 "if(1==1){a=1;return a;}return 0;"
try 6 "i=0;while(i<5){i=i+1;i=i+1;}return i;"
try 2 "i=0;{i=1;i=2;}return i;"



echo OK
