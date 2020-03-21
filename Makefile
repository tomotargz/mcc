CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS := $(SRCS:%.c=%.o)
DEPS := $(SRCS:%.c=%.d)

mcc: $(OBJS)
	$(CC) -o mcc $(OBJS) $(LDFLAGS)

-include $(DEPS)

%.o: %.c
	$(CC) -c -MMD -MP $<

test: mcc
	./test.sh

clean:
	rm -f mcc *.o *~ tmp*

.PHONY: test clean
