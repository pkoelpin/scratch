#pragma once
#include <windows.h>

HWND toolbar_create(HWND hwnd_parent, HINSTANCE hInstance);
void toolbar_notify(HINSTANCE hInst, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);