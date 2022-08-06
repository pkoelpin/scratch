#pragma once
#include <wchar.h>

#define VIS_CLEAR  1
#define VIS_SHOW   2
#define VIS_HIDE   3

typedef struct entitylist entitylist;

entitylist* entitylist_create();
void entitylist_free(entitylist *el);
void entitylist_setall(entitylist* el, int count, int* id, int* visibility, wchar_t** title);
void entitylist_get(entitylist* el, int index, int* id, int* visibility, wchar_t** title);
int  entitylist_count(entitylist* el);
void entitylist_vis_advance(entitylist* el, int index);
int  entitylist_get_vis_count(entitylist* el);
void entitylist_get_vis(entitylist* el, int* nGroupID);
void entitylist_filter(entitylist* el, wchar_t* pat);
