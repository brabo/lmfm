CC=gcc

CFLAGS=-I. --std=gnu99 -I./include -fms-extensions

FAT_OBJ=src/mockblock.o src/fat.o

LDFLAGS=
LDLIBS=


fat: $(FAT_OBJ)
	$(CC) -o $@ $(FAT_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f $(FAT_OBJ)

all: fat

clean:
	rm -f src/*.o fat
