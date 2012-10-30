CFLAGS=-O2
LDFLAGS=-lbluetooth -lcwiid -lSDL -lSDL_ttf
EXEC=test_wii pointer

all: ${EXEC}

pointer: pointer.o
	gcc -o pointer pointer.o ${LDFLAGS}

pointer.o: pointer.c
	gcc -o pointer.o -c pointer.c ${CFLAGS}

test_wii: test_wii.o
	gcc -o test_wii test_wii.o ${LDFLAGS}

test_wii.o: test_wii.c
	gcc -o test_wii.o -c test_wii.c ${CFLAGS}

clean:
	rm -rf *.o

mrproper: clean
	rm -rf ${EXEC}
