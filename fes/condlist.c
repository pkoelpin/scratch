#include <windows.h>
#include <stdbool.h>
#include "condlist.h"

/*
spmatch - String Pattern Match
https://dogankurt.com/wildcard.html

returns 1 for match
*/
static int spmatch(const char *pat, const char *str)
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

static void long2str(unsigned char *buf, long x) {
    unsigned char tmp[64];
    unsigned char *end = tmp + sizeof(tmp);
    unsigned char *beg = end;
    long t = x>0 ? -x : x;
    do {
        *--beg = '0' - t%10;
    } while (t /= 10);
    for (int i = 0; i < end-beg; i++) {
        buf[i] = beg[i];
    }
    buf[end-beg] = '\0';
}

struct condlist {
    int *id;
    const char * const *title;
    int n;
    int *idx;
    int len;
    bool isactive;
};

condlist *condlist_create(int *id, const char * const *title, int n, bool isactive) {
    struct condlist *list = (struct condlist*)HeapAlloc(GetProcessHeap(), 0, sizeof(struct condlist));
    list->id = id;
    list->title = title;
    list->n = n;
    list->idx = (int*)HeapAlloc(GetProcessHeap(), 0, n*sizeof(int));
    list->len = 0;
    list->isactive = isactive;
    condlist_update(list, NULL);
    return list;
}

void condlist_free(condlist *list) {
    HeapFree(GetProcessHeap(), 0, list->idx);
    HeapFree(GetProcessHeap(), 0, list);
}

void condlist_update(condlist *list, const char *filter) {
    list->len = 0;
    for (int i=0; i<list->n; i++) {
        if ((list->id[i] > 0) == list->isactive)  {
            char num[64];
            long2str(num, list->id[i]);
            if ( (filter == NULL) || 
                 (filter[0] == '\0') || 
                 (spmatch(filter, list->title[i])) ||
                 (spmatch(filter, num))
                ) 
            {
                list->idx[list->len++] = i;
            }
        }
    }
}

int condlist_get_id(condlist *list, int index) {
    return list->id[list->idx[index]];
}

const char * condlist_get_title(condlist *list, int index) {
    return list->title[list->idx[index]];
}

void condlist_flip(condlist *list, int index) {
     list->id[list->idx[index]] = -list->id[list->idx[index]];
}

int condlist_len(condlist *list) {
    return list->len;
}

