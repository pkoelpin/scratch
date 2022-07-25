#pragma once
#include <windows.h>
#include "entitylist.h"

HWND listview_create(HWND hwnd_parent, HINSTANCE hInstance);
void listview_notify(HINSTANCE hInst, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, entitylist *el);
