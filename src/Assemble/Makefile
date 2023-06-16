CC	=	gcc
CFLAGS	=	-Wall -g
BUILD	=	assemble

all:	$(BUILD)

clean:
	/bin/rm -f *.o assemble

assemble: assemble.o dynmap.o ../BitUtils/custombit.o assembleBranches.o assembleDPI.o assembleSingleDTI.o

assemble.o: dynmap.h assembleBranches.h assembleDPI.h assembleSingleDTI.h

assembleBranches.o: assembleBranches.h

assembleDPI.o: assembleDPI.h ../BitUtils/custombit.o

assembleSingleDTI.o: assembleSingleDTI.h dynmap.h

dynmap.o: dynmap.h

