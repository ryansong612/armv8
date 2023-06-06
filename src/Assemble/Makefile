CC	=	gcc
CFLAGS	=	-Wall
BUILD	=	assemble

all:	$(BUILD)

clean:
	/bin/rm -f $(BUILD) *.o core

assemble: assemble.o dynmap.o

assemble.o: dynmap.h

dynmap.o: dynmap.h


