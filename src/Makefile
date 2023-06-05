CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\

.SUFFIXES: .c .o
.PHONY: all clean

all: assemble emulate

assemble: assemble.o

emulate: emulate.o

clean:
	$(RM) *.o assemble emulate
	
emulate:       emulate.o custombit.o dpi-immediate.o dpi-register.o parseLS.o parseBranches.o readnwrite.o

custombit.o: custombit.h

readnwrite.o: readnwrite.h emulate.h

parseBranches.o: parseBranches.h emulate.h readnwrite.h custombit.h

parseLS.o: parseLS.h emulate.h readnwrite.h custombit.h

dpi-register.o: dpi-register.h emulate.h readnwrite.h custombit.h

dpi-immediate.o: dpi-immediate.h emulate.h readnwrite.h custombit.h

emulate.o: dpi-immediate.h dpi-register.h parseLS.h parseBranches.h emulate.h readnwrite.h custombit.h

