#ifdef _MSC_VER
  #pragma comment(linker, "/subsystem:console")
#endif

#include <windows.h>
#include <stdio.h>
#include "fepick.h"

int mainCRTStartup(void) {
    int n = 100000;
    int *id = HeapAlloc(GetProcessHeap(), 0, n*sizeof(int));
    char **title = HeapAlloc(GetProcessHeap(), 0, n*sizeof(char *));
    static char subcase[] = "subcase";
    for (int i = 0; i < n; i++) {
        id[i] = -(i+1);
        title[i] = HeapAlloc(GetProcessHeap(), 0, 30*sizeof(char));
        title[i][0] = 0xE2;
        title[i][1] = 0x98;
        title[i][2] = 0xBA;
        title[i][3] = '\0';
        //memcpy(title[i], subcase, sizeof(subcase)) ;
    }
    int count = fepick_case(id, title, n);
    printf("%d", count);
    //printf("%d", GetLastError());

    ExitProcess(0);
}