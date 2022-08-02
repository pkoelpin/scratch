#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

void* femap_connect();
long femap_hMainWnd(void* model);
void femap_register(void* model, int hwnd);

int  femap_group_CountSet(void* model);
void femap_group_GetTitleList(void* model, int* id, wchar_t** title);
void femap_group_GetVisibility(void* model, const int* id, int* visibility);
void femap_view_SetMultiGroupList(void* model, bool bClear, int nGroups, const int* nGroupID);

#ifdef __cplusplus
}
#endif