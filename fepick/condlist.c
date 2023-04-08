/* The pragma supresses warnings from the standard headers */
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
    if (str == NULL) {
        return 0;
    }

    const char *locp = NULL;
    const char *locs = NULL;

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

/* Convert a long to a string */
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
    bool isactive;
    int *id;         /* external pointer */
    char **title;    /* external pointer */
    int n;           /* length of the external pointers */
    int *idx_sorted; /* this will always hold n */
    int *idx;        /* this holds the data for the list view*/
    int len;         /* total in idx array. this is after filtering */
    int count;       /* total that pass the bool test for activity */
};

condlist *condlist_create(int *id, const char * const *title, int n, bool isactive) {
    struct condlist *list = (struct condlist*)HeapAlloc(GetProcessHeap(), 0, sizeof(struct condlist));
    list->id = id;
    list->title = title;
    list->n = n;
    list->idx = (int*)HeapAlloc(GetProcessHeap(), 0, n*sizeof(int));
    list->idx_sorted = (int*)HeapAlloc(GetProcessHeap(), 0, n*sizeof(int));
    list->len = 0;
    list->isactive = isactive; 
    /* Set the initial sorted indexing */
    for (int i=0; i < list->n; i++){
        list->idx_sorted[i] = i;
    }
    //condlist_sort_id_ascending(list);
    condlist_update(list, NULL);
    return list;
}

void condlist_free(condlist *list) {
    HeapFree(GetProcessHeap(), 0, list->idx);
    HeapFree(GetProcessHeap(), 0, list->idx_sorted);
    HeapFree(GetProcessHeap(), 0, list);
}

/* The 'i' is the index in the original list */
static const char *condlist_title(condlist *list, int i) {
    if ((list->title == NULL) ||  (list->title[i] == NULL)) {
        return '\0';
    } else {
        return list->title[i];
    }
}

static int compare_id_ascending( void *vlist, const void *val1, const void *val2) {
    condlist *list = (condlist *)vlist;
    int a = abs(list->id[*(int*)val1]);
    int b = abs(list->id[*(int*)val2]);
    return (a > b) - (a < b);
}

static int compare_id_descending( void *vlist, const void *val1, const void *val2) {
    condlist *list = (condlist *)vlist;
    int a = abs(list->id[*(int*)val1]);
    int b = abs(list->id[*(int*)val2]);
    return (a < b) - (a > b);
}

static int compare_title_ascending( void *vlist, const void *val1, const void *val2) {
    condlist *list = (condlist *)vlist;
    const char *a = condlist_title(list, *(int*)val1);
    const char *b = condlist_title(list, *(int*)val2);
    return strcmp(a,b);
}

static int compare_title_descending( void *vlist, const void *val1, const void *val2) {
    condlist *list = (condlist *)vlist;
    const char *a = condlist_title(list, *(int*)val1);
    const char *b = condlist_title(list, *(int*)val2);
    return -strcmp(a,b);
}

void condlist_sort_clear(condlist *list) {
    /* Set the initial sorted indexing */
    for (int i=0; i < list->n; i++){
        list->idx_sorted[i] = i;
    }
}

void condlist_sort_id_ascending(condlist *list) {
    qsort_s(list->idx_sorted, list->n, sizeof(int), compare_id_ascending, list);
}

void condlist_sort_id_descending(condlist *list) {
    qsort_s(list->idx_sorted, list->n, sizeof(int), compare_id_descending, list);
}

void condlist_sort_title_ascending(condlist *list) {
    qsort_s(list->idx_sorted, list->n, sizeof(int), compare_title_ascending, list);
}

void condlist_sort_title_descending(condlist *list) {
    qsort_s(list->idx_sorted, list->n, sizeof(int), compare_title_descending, list);
}

void condlist_update(condlist *list, const char *filter) {
    list->len = 0;
    list->count = 0;

    int *s = list->idx_sorted;

    for (int i=0; i<list->n; i++) {
        if (list->id[s[i]] == 0) continue; /* skip 0 */
        if ((list->id[s[i]] > 0) == list->isactive)  {
            list->count++;
            char num[64];
            long2str(num, list->id[s[i]]);
            if ( (filter == NULL) || 
                 (filter[0] == '\0') || 
                 (spmatch(filter, condlist_title(list,s[i]))) ||
                 (spmatch(filter, num))
                ) 
            {
                list->idx[list->len++] = s[i];
            }
        }
    }
}

/* Update the base pointer */
void condlist_set_id(condlist *list, int *id) {
    list->id = id;
}

/* the index is for the listview */
int condlist_get_id(condlist *list, int index) {
    return list->id[list->idx[index]];
}

const char* condlist_get_title(condlist *list, int index) {
    return condlist_title(list, list->idx[index]);
}

void condlist_flip(condlist *list, int index) {
     list->id[list->idx[index]] = -list->id[list->idx[index]];
}

int condlist_len(condlist *list) {
    return list->len;
}

int condlist_count(condlist *list) {
    return list->count;
}

