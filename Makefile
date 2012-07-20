CC=gcc
CFLAGS=-O3 -Wall -Werror
LDFLAGS=-lpthread

.PHONY: all clean

all: sysres

sysres: main.o print_funcs.o mem_tracker.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

clean:
	rm *.o
	rm sysres
