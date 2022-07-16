/*
spmatch - String Pattern Match
https://dogankurt.com/wildcard.html
*/
#include <stddef.h>

int spmatch(char *pat, char *str)
{
    char *locp = NULL;
    char *locs = NULL;

    while (*str) {
        /* we encounter a star */
        if (*pat == '*') {
            locp = ++pat;
            locs = str;
            if (*pat == '\0') {
                return 1;
            }
            continue;
        }
        /* we have a mismatch */
        if (*str != *pat && *pat != '?') {
            if (!locp) {
                return 0;
            }
            str = ++locs;
            pat = locp;
            continue;
        }
        pat++, str++;
    }
    /* check if the pattern's ended */
    while (*pat == '*') {
        pat++;
    }
    return (*pat == '\0');
}

int main() {
    return 0;
}