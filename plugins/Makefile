CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -fPIC -shared
LIBS = -lncurses

SRCS = $(wildcard *.c)
TARGETS = $(SRCS:.c=.so)

all: $(TARGETS)

%.so: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f *.so



.PHONY: all clean