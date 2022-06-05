.POSIX:
CROSS   =
CC      = $(CROSS)gcc
CFLAGS  = -std=c99 -DNDEBUG -ffast-math -Os
LDFLAGS = 
LDLIBS  = 

D_CFLAGS  = -Wall -Wextra -Wdouble-promotion -g -Og
D_LDFLAGS = -mconsole

bdf.exe: bdf.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ bdf.c $(LDLIBS)

all: bdf.exe debug.exe

debug.exe: bdf.c
	$(CC) $(D_CFLAGS) $(D_LDFLAGS) -o $@ bdf.c $(LDLIBS)

clean:
	rm -f debug.exe bdf.exe