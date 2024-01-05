CC=gcc
CFLAGS=-std=c99 -Wall -Og -g

all: lz77

lz77: lz77.c
	$(CC) $(CFLAGS) lz77.c -o lz77

helper: helper.c
	$(CC) $(CFLAGS) helper.c -o helper
	./helper

clean:
	rm -f lz77 helper
