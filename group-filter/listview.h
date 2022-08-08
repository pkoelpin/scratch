#pragma once
#include <windows.h>
#include "entitylist.h"

HWND listview_create(HWND hwnd_parent, HINSTANCE hInstance);
int listview_notify(HINSTANCE hInst, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, void* model, entitylist *el, HWND hwnd_statubar);
