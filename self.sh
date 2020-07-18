#!/bin/bash -eu

TMP=tmp-self
mkdir -p $TMP

compileWithMcc2(){
    file=$1
    ./preproc.sh $file $TMP
    ./mcc $TMP/$file > $TMP/${file%.c}.s
    gcc -c -o $TMP/${file%.c}.o $TMP/${file%.c}.s
}

for i in ./*.c; do
    gcc -I. -c -o $TMP/${i%.c}.o $i
done

compileWithMcc2 codegen.c compileWithMcc2 error.c
compileWithMcc2 main.c
compileWithMcc2 node.c
compileWithMcc2 parse.c
compileWithMcc2 tokenize.c
compileWithMcc2 type.c

gcc -static -o mcc2 $TMP/*.o
