#include <stdlib.h>
#include "entitylist.h"

int match(wchar_t* pat, wchar_t* str);

typedef struct entity
{
    int      id;
    int      visibility;
    wchar_t* title;
} entity;

struct entitylist
{
    int      count;
    entity*  list;
    int      count_sorted;
    entity** list_sorted;
    int      count_filtered;
    entity** list_filtered;
};

entitylist* entitylist_create()
{
    entitylist* el = malloc(sizeof(entitylist));
    if (!el)
        return NULL;
    el->count = 0;
    el->list = NULL;
    el->count_sorted = 0;
    el->list_sorted = NULL;
    el->count_filtered = 0;
    el->list_filtered = NULL;
    return el;
}

void entitylist_free(entitylist* el)
{
    free(el->list_sorted);
    free(el->list_filtered);
    for (int i = 0; i < el->count; i++)
    {
        free(el->list[i].title);
    }
    free(el->list);
    free(el);
}

void entitylist_get(entitylist* el, int index, int* id, int* visibility, wchar_t** title)
{
    if (id != NULL)
        *id = el->list_filtered[index]->id;
    if (visibility != NULL)
        *visibility = el->list_filtered[index]->visibility;
    if (title != NULL)
        *title = el->list_filtered[index]->title;
}

void entitylist_set_vis(entitylist* el, int index, int visibility)
{
    el->list_filtered[index]->visibility = visibility;
}

void entitylist_setall(entitylist* el, int count, int* id, int * visibility, wchar_t** title)
{
    if (count > el->count) {
        el->list = realloc(el->list, count * sizeof(entity));
        el->list_sorted = realloc(el->list_sorted, count * sizeof(entity*));
        el->list_filtered = realloc(el->list_filtered, count * sizeof(entity*));
    }
    
    el->count = count;
    el->count_sorted = count;
    el->count_filtered = count;
    
    for (int i = 0; i < count; i++)
    {
        el->list[i].id = id[i];
        el->list[i].visibility = visibility[i];
        
        size_t len = wcslen(title[i]) + 1;
        el->list[i].title = malloc(len * sizeof(wchar_t));
        memcpy(el->list[i].title, title[i], len * sizeof(wchar_t) );

        el->list_sorted[i] = &(el->list[i]);
        el->list_filtered[i] = &(el->list[i]);
    }
}

int entitylist_count(entitylist* el)
{
    return el->count_filtered;
}

int entitylist_get_vis_count(entitylist* el)
{
    int count = 0;
    for (int i = 0; i < el->count; i++)
    {
        if (el->list[i].visibility != VIS_CLEAR)
        {
            count++;
        }
    }
    return count;
}

void entitylist_get_vis(entitylist* el, int* nGroupID)
{
    int count = 0;
    for (int i = 0; i < el->count; i++)
    {
        if (el->list[i].visibility == VIS_SHOW)
        {
            nGroupID[count++] = el->list[i].id;
        }
        else if (el->list[i].visibility == VIS_HIDE)
        {
            nGroupID[count++] = -el->list[i].id;
        }
    }
}

void entitylist_vis_advance(entitylist* el, int index)
{
    switch (el->list_filtered[index]->visibility)
    {
    case VIS_CLEAR:
        el->list_filtered[index]->visibility = VIS_SHOW;
        break;
    case VIS_SHOW:
        el->list_filtered[index]->visibility = VIS_HIDE;
        break;
    case VIS_HIDE:
        el->list_filtered[index]->visibility = VIS_CLEAR;
        break;
    }
}

void entitylist_filter(entitylist* el, wchar_t* pat)
{
    el->count_filtered = 0;
    int test = match(pat, el->list_sorted[0]->title);
    free(NULL);
    for (int i = 0; i < el->count_sorted; i++)
    {
        if (match(pat, el->list_sorted[i]->title))
        {
            el->list_filtered[el->count_filtered++] = el->list_sorted[i];
        }
    }
}

int match(wchar_t* pat, wchar_t* str)
{
    wchar_t* locp = NULL;
    wchar_t* locs = NULL;

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