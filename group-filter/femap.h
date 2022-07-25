#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void* femap_connect();
long femap_hMainWnd(void* model);
void femap_register(void* model, int hwnd);

int  femap_group_CountSet(void* model);
void femap_group_GetTitleList(void* model, int* id, wchar_t** title);
void femap_group_GetVisibility(void* model, const int* id, int* visibility);

#ifdef __cplusplus
}
#endif