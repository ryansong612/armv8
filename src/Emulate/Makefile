CC	=	gcc
CFLAGS	=	-Wall
BUILD	=	emulate

all:	$(BUILD)

clean:
	/bin/rm -f *.o emulate

emulate: emulate.o custombit.o dpi-immediate.o dpi-register.o parseLS.o parseBranches.o readnwrite.o

custombit.o: custombit.h

readnwrite.o: readnwrite.h emulate.h

parseBranches.o: parseBranches.h emulate.h readnwrite.h custombit.h

parseLS.o: parseLS.h emulate.h readnwrite.h custombit.h

dpi-register.o: dpi-register.h emulate.h readnwrite.h custombit.h

dpi-immediate.o: dpi-immediate.h emulate.h readnwrite.h custombit.h

emulate.o: dpi-immediate.h emulate.h parseLS.h parseBranches.h emulate.h readnwrite.h custombit.h


