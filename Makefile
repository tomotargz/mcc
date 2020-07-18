CFLAGS=-std=c11 -g -O0 -static -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

$(OBJS): mcc.h

extern.o: test/extern.c
	gcc -c -o extern.o test/extern.c

mcc: $(OBJS)
	$(CC) -o mcc $(OBJS) $(LDFLAGS)

test: mcc extern.o
	./mcc "test/test.c" > tmp.s
	gcc -static -o tmp tmp.s extern.o
	./tmp

mcc2: mcc $(SRCS) mcc.h
	./self.sh

test2: mcc2 extern.o
	./mcc2 "test/test.c" > tmp.s
	gcc -static -o tmp tmp.s extern.o
	./tmp

chibicc:
	./chibicc.sh

clean:
	rm -rf mcc mcc2 *.o *~ tmp*

.PHONY: test test2 clean chibicc
