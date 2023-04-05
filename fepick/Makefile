CFLAGS  = /O2 /nologo /W3

all: test.exe fepick.dll fepick.lib

fepick.dll: fepick.obj fepick.res condlist.obj
	link $**  /NOLOGO /DLL /NODEFAULTLIB /out:$@ kernel32.lib user32.lib Gdi32.lib vcruntime.lib libcmt.lib

fepick.lib: fepick.dll

test.exe: test.c fepick.h fepick.lib
	$(CC) $(CFLAGS) test.c fepick.lib ucrt.lib kernel32.lib

fepick.res: manifest.xml resource.h fepick.rc ./icons/favicon.ico
	$(RC) /nologo /fo $@ fepick.rc

fepick.obj: fepick.c resource.h condlist.h strbuf.h
	$(CC) $(CFLAGS) /c /Fo:$@ fepick.c

condlist.obj: condlist.c condlist.h
	$(CC) $(CFLAGS) /c /Fo:$@ condlist.c

clean:
	del fepick.obj fepick.lib fepick.dll fepick.exp fepick.res test.exe test.obj condlist.obj