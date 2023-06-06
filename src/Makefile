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
	
emulate: Emulate/emulate.o Emulate/custombit.o Emulate/dpi-immediate.o Emulate/dpi-register.o Emulate/parseLS.o Emulate/parseBranches.o Emulate/readnwrite.o

Emulate/custombit.o: Emulate/custombit.h

Emulate/readnwrite.o: Emulate/readnwrite.h Emulate/emulate.h

Emulate/parseBranches.o: Emulate/parseBranches.h Emulate/emulate.h Emulate/readnwrite.h Emulate/custombit.h

Emulate/parseLS.o: Emulate/parseLS.h Emulate/emulate.h Emulate/readnwrite.h Emulate/custombit.h

Emulate/dpi-register.o: Emulate/dpi-register.h Emulate/emulate.h Emulate/readnwrite.h Emulate/custombit.h

Emulate/dpi-immediate.o: Emulate/dpi-immediate.h Emulate/emulate.h Emulate/readnwrite.h Emulate/custombit.h

Emulate/emulate.o: Emulate/dpi-immediate.h Emulate/dpi-register.h Emulate/parseLS.h Emulate/parseBranches.h Emulate/emulate.h Emulate/readnwrite.h Emulate/custombit.h

dynmap.o: Assemble/dynmap.h

