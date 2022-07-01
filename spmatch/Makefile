.POSIX:
CROSS   =
CC      = $(CROSS)gcc
CFLAGS  = -std=c99 -DNDEBUG -ffast-math -Os
LDFLAGS = 
LDLIBS  = 

D_CFLAGS  = -Wall -Wextra -Wdouble-promotion -g -Og
D_LDFLAGS = -mconsole

spmatch.exe: spmatch.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ spmatch.c $(LDLIBS)

all: spmatch.exe debug.exe

debug.exe: spmatch.c
	$(CC) $(D_CFLAGS) $(D_LDFLAGS) -o $@ spmatch.c $(LDLIBS)

clean:
	rm -f debug.exe spmatch.exe