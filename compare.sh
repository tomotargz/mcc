#!/bin/bash -e

if [ $# -ne 1 ]; then
    echo need a file name as an argument
    exit 1
fi

FILE=$1
TMP=tmp-compare
mkdir -p $TMP
./preproc.sh $FILE $TMP
./mcc $TMP/$FILE > $TMP/$FILE.mcc
./chibicc/chibicc $TMP/$FILE > $TMP/$FILE.chibicc
diff -u $TMP/$FILE.mcc $TMP/$FILE.chibicc > $TMP/$FILE.diff || true
less $TMP/$FILE.diff
