CFLAGS=-std=c11 -g3 -O0 -static
SRCS=$(wildcard *.c)
HDRS=$(wildcard *.h)
OBJS := $(SRCS:%.c=%.o)
DEPS := $(SRCS:%.c=%.d)

mcc: $(OBJS)
	$(CC) -o mcc $(OBJS) $(LDFLAGS)

mcc2: mcc $(SRCS) $(HDRS) self.sh
	./self.sh

-include $(DEPS)

%.o: %.c
	$(CC) $(CFLAGS) -c -MMD -MP $<

test: mcc
	./mcc "test/test.c" > tmp.s
	gcc -static -O0 -o tmp tmp.s
	./tmp

test2: mcc2
	./mcc2 "test/test.c" > tmp2.s
	gcc -static -O0 -o tmp2 tmp2.s
	./tmp2

clean:
	rm -rf mcc mcc2 *.o *~ tmp*

.PHONY: test clean
