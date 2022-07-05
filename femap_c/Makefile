.POSIX:
.SUFFIXES:
CC      = gcc
CFLAGS  = -std=c99 -ffast-math -Os -Wall -Wextra -Wdouble-promotion
LDFLAGS = 
LDLIBS  = -lole32 -loleaut32 -luuid

all: filter.exe

filter.exe: main.o model.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
main.o: main.c model.h
model.o: model.c model.h
clean:
	rm -f filter.exe

.SUFFIXES: .c .o
	$(CC) $(CFLAGS) -c $<