CC	=	gcc
BUILD	=	libs

all:	$(BUILD)

clean:
	/bin/rm -f $(BUILD) *.o core
	cd Emulate; make clean; cd ../Assemble; make clean

libs:
	cd Emulate; make; cd ../Assemble; make