CFLAGS=-std=c11 -g3 -O0 -static
SRCS=$(wildcard *.c)
OBJS := $(SRCS:%.c=%.o)
DEPS := $(SRCS:%.c=%.d)

mcc: $(OBJS)
	$(CC) -o mcc $(OBJS) $(LDFLAGS)

-include $(DEPS)

%.o: %.c
	$(CC) $(CFLAGS) -c -MMD -MP $<

test: mcc
	./test.sh

clean:
	rm -f mcc *.o *~ tmp*

.PHONY: test clean
