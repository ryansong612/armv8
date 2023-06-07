CC	=	gcc
CFLAGS	=	-Wall
BUILD	=	assemble

all:	$(BUILD)

clean:
	/bin/rm -f *.o assemble

assemble: assemble.o dynmap.o ../BitUtils/custombit.o

assemble.o: dynmap.h

dynmap.o: dynmap.h
