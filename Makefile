CC=gcc
CFLAGS=-O3
LDFLAGS=-lpthread

.PHONY: all clean

all: mem_free

mem_free: mem_free.o 
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

clean:
	rm *.o
	rm mem_free
