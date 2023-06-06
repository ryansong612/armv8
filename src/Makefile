CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\

.SUFFIXES: .c .o
.PHONY: all clean

all: assemble emulate

assemble: Assemble/assemble.o

emulate: Emulate/emulate.o

clean:
	$(RM) *.o assemble emulate
	
emulate:       Emulate/emulate.o custombit.o dpi-immediate.o dpi-register.o parseLS.o parseBranches.o readnwrite.o

custombit.o: Emulate/custombit.h

readnwrite.o: Emulate/readnwrite.h emulate.h

parseBranches.o: Emulate/parseBranches.h emulate.h readnwrite.h custombit.h

parseLS.o: Emulate/parseLS.h emulate.h readnwrite.h custombit.h

dpi-register.o: Emulate/dpi-register.h emulate.h readnwrite.h custombit.h

dpi-immediate.o: Emulate/dpi-immediate.h emulate.h readnwrite.h custombit.h

emulate.o: Emulate/dpi-immediate.h dpi-register.h parseLS.h parseBranches.h emulate.h readnwrite.h custombit.h

