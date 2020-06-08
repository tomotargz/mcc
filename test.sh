#!/bin/bash

test() {
    input="$1"
    ./mcc "$input" > tmp.s
    gcc -static -O0 -o tmp tmp.s
    ./tmp
}

test "test/test.c"
exit 0
