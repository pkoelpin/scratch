#include <stdlib.h>
#include "entitylist.h"

typedef struct entity
{
    int      id;
    int      visibility;
    wchar_t* title;
} entity;

struct entitylist
{
    int       count;
    entity* list;
    int       count_sorted;
    entity** list_sorted;
    int       count_filtered;
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
    *id = el->list_filtered[index]->id;
    *visibility = el->list_filtered[index]->visibility;
    *title = el->list_filtered[index]->title;
}


void entitylist_set(entitylist* el, int count, int* id, int * visibility, wchar_t** title)
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
