#!/bin/bash -eu

TMP=tmp-test-self

test(){
    FILE=$1
    ./preproc.sh $FILE $TMP
    ./mcc $TMP/$FILE > $TMP/$FILE.mcc
    ./mcc2 $TMP/$FILE > $TMP/$FILE.mcc2
    diff -u $TMP/$FILE.mcc $TMP/$FILE.mcc2
    echo $FILE is OK
}

mkdir -p $TMP
make mcc
make mcc2
test main.c
test tokenize.c
test parse.c
test codegen.c
test type.c
test node.c
test error.c

echo OK
