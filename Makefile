CFLAGS=-std=c11 -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
		$(CC) -o 9cc $(OBJS) $(LDFLAGS)

test: 9cc
		./test.sh

clean:
		rm -f 9cc *.o *~ tmp*

clear:
		rm -f 9cc *.o *~ tmp*


.PHONY: test clean clear

