.PHONY: all clean

#Important Directories
BINDIR=./bin
SRCDIR=./src

#Compilation Vars
CC=gcc
CFLAGS=-O3 -Wall -Werror
LDFLAGS=-lpthread

#List of source files 
SRC_FILES=main.c print_funcs.c mem_tracker.c
#List of generated object files
OBJS = $(patsubst %.c, $(BINDIR)/%.o, $(SRC_FILES))
#Final executable
EXE=$(BINDIR)/sysres

#Default Rule
all: $(EXE)

#Rule to make the generated file directory
$(BINDIR):
	mkdir -p $(BINDIR)

#Rule to make object files
$(BINDIR)/%.o: $(SRCDIR)/%.c | $(BINDIR)
	$(CC) -c $(CFLAGS) $< -o $@

#Rule to make final executable
$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

#Rule to clean the build
clean:
	-rm -rf $(BINDIR)
