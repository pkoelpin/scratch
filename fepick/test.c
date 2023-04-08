#ifdef _MSC_VER
  #pragma comment(linker, "/subsystem:console")
#endif

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "fepick.h"

void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 2);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

int mainCRTStartup(void) {
    int n = 100000;
    int *id = HeapAlloc(GetProcessHeap(), 0, n*sizeof(int));
    char **title = HeapAlloc(GetProcessHeap(), 0, n*sizeof(char *));
    static char subcase[] = "subcase";
    for (int i = 0; i < n; i++) {
        id[i] = -((rand()*rand() % n) + 1);
        title[i] = HeapAlloc(GetProcessHeap(), 0, 30*sizeof(char));
        rand_str(title[i], 20);
        //memcpy(title[i], subcase, sizeof(subcase)) ;
    }
    int count = fepick_case(id, title, n);
    printf("%d", count);
    ExitProcess(0);
}