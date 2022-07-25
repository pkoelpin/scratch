#pragma once
#include <wchar.h>

#define VIS_CLEAR  1
#define VIS_SHOW   2
#define VIS_HIDE   3

typedef struct entitylist entitylist;

entitylist* entitylist_create();
void entitylist_free(entitylist *el);
void entitylist_set(entitylist* el, int count, int* id, int* visibility, wchar_t** title);
void entitylist_get(entitylist* el, int index, int* id, int* visibility, wchar_t** title);
void entitylist_vis_advance(entitylist* el, int index);
//void entitylist_filter(entitylist* el, wchar_t* s);
