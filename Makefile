CC=gcc

CFLAGS=-I. --std=gnu99 -I./include -fms-extensions -fsanitize=address -m32 -O0 -ggdb3 -pg

LMFM_OBJ=src/mockblock.o src/module.o src/vfs.o src/fatfs.o src/binutils.o src/lmfm.o

LDFLAGS=
LDLIBS=


lmfm: $(LMFM_OBJ)
	$(CC) -o $@ $(LMFM_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f $(LMFM_OBJ)

all: lmfm

clean:
	rm -f src/*.o lmfm
