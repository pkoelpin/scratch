CFLAGS  = /O2

ktk.exe: ktk.obj ktk.res
	link /out:$@ ktk.obj ktk.res

ktk.res: manifest.xml resource.h ktk.rc favicon.ico
	rc /fo $@ ktk.rc

ktk.obj: ktk.c resource.h
	cl $(CFLAGS) /Fo:$@ ktk.c

clean:
	del ktk.obj ktk.exe ktk.res