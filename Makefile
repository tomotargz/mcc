CFLAGS=-std=c11 -g -O0 -static -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

$(OBJS): mcc.h

mcc: $(OBJS)
	$(CC) -o mcc $(OBJS) $(LDFLAGS)

test: mcc
	./mcc "test/test.c" > tmp.s
	gcc -static -o tmp tmp.s
	./tmp

mcc2: mcc $(SRCS) mcc.h
	./self.sh

test2: mcc2
	./mcc2 "test/test.c" > tmp.s
	gcc -static -o tmp tmp.s
	./tmp

clean:
	rm -rf mcc mcc2 *.o *~ tmp*

.PHONY: test clean
