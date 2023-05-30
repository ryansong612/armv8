CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic

.SUFFIXES: .c .o

.PHONY: all clean

all: parse.c

#assemble: assemble.o
#emulate: emulate.o
parse: parse.o

clean:
	$(RM) *.o parse #assemble emulate
	
