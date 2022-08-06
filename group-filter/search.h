#pragma once
#include <windows.h>
#include "entitylist.h"

HWND search_create(HWND hwnd_parent, HINSTANCE hInstance);
void search_command(HINSTANCE hInst, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, entitylist* el);