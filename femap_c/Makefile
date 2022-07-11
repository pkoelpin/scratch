.POSIX:
.SUFFIXES:
CC      = gcc
CFLAGS  = -std=c99 -ffast-math -Os -Wall -Wextra -Wdouble-promotion
LDFLAGS = 
LDLIBS  = -lole32 -loleaut32 -luuid -lcomctl32 -luxtheme

all: filter.exe

filter.exe: main.o model.o lcpick.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
main.o: main.c model.h
model.o: model.c model.h
lcpick.o: lcpick.c lcpick.h
clean:
	rm -f filter.exe *.o

.SUFFIXES: .c .o
	$(CC) $(CFLAGS) -c $<