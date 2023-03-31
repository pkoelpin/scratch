CFLAGS  = /O2 /nologo

all: test.exe fes.dll fes.lib

fes.dll: fes.obj fes.res condlist.obj
	link $** /NOLOGO /DLL /NODEFAULTLIB /out:$@ kernel32.lib user32.lib vcruntime.lib libcmt.lib

fes.lib: fes.dll

test.exe: test.c fes.h fes.lib
	$(CC) $(CFLAGS) test.c fes.lib ucrt.lib kernel32.lib

fes.res: manifest.xml resource.h fes.rc
	$(RC) /nologo /fo $@ fes.rc

fes.obj: fes.c resource.h
	$(CC) $(CFLAGS) /c /Fo:$@ fes.c

condlist.obj: condlist.c condlist.h
	$(CC) $(CFLAGS) /c /Fo:$@ condlist.c

clean:
	del fes.obj fes.lib fes.dll fes.exp fes.res test.exe test.obj condlist.obj