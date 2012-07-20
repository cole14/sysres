CC=gcc
CFLAGS=-O3
LDFLAGS=-lpthread

.PHONY: all clean

all: sysres

sysres: mem_free.o print_funcs.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

clean:
	rm *.o
	rm sysres
