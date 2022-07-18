#include "Resource.h"
#include <Windows.h>
#include <commctrl.h>

/*
Show Full Model, Show Active Group, Show Multiple Groups
-
Show Selected Groups Only
Show Selected Groups
Hide Selected Groups
Clear Selected Groups
-
Show All Groups
Clear All Groups
-
Group Select
Element Select
-
Highlight
Reload
Help

*/

LRESULT CALLBACK WndProc_toolbar(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

HWND toolbar_create(HWND hwnd_parent, HINSTANCE hInstance)
{
    const int ImageListID = 0;
    const int numButtons  = 11;
    const int bitmapSize  = 16;

    HWND hwnd_toolbar = CreateWindowEx(
        0,
        TOOLBARCLASSNAME,
        NULL,
        WS_CHILD | TBSTYLE_WRAPABLE | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
        0, 0, 0, 0,
        hwnd_parent, NULL, hInstance, NULL);

    if (hwnd_toolbar == NULL)
        return NULL;
    SetWindowSubclass(hwnd_toolbar, &WndProc_toolbar, 1, 0);

    SendMessage(hwnd_toolbar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
    SendMessage(hwnd_toolbar, TB_SETMAXTEXTROWS, 0, 0);


    HIMAGELIST g_hImageList = ImageList_Create(
        bitmapSize, 
        bitmapSize,  
        ILC_COLOR24 | ILC_MASK,
        numButtons,
        0);

    // Set the image list.
    SendMessage(
        hwnd_toolbar, 
        TB_SETIMAGELIST,
        (WPARAM)ImageListID,
        (LPARAM)g_hImageList);

    int bitmaps[] = { 
        IDB_SHOW_FULL_MODEL,
        IDB_SHOW_SELECTED_ONLY, 
        IDB_SHOW_SELECTED, 
        IDB_HIDE_SELECTED,
        IDB_CLEAR_SELECTED,
        IDB_SHOW_ALL,
        IDB_CLEAR_ALL,
        IDB_GROUP,
        IDB_ELEMENT,
        IDB_RELOAD,
        IDB_HIGHLIGHT};
    HBITMAP hbmp;

    for (int i = 0; i < numButtons; i++) {
        hbmp = LoadBitmap(hInstance, MAKEINTRESOURCE(bitmaps[i]));
        ImageList_AddMasked(g_hImageList, hbmp, RGB(233, 236, 238));
    }

    TBBUTTON tbButtons[] = 
    {
        {0, IDB_SHOW_FULL_MODEL, TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN, {0}, 0, L""},
        {10, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, -1},
        {1, IDB_SHOW_SELECTED_ONLY, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Show Selected Groups Only"},
        {2, IDB_SHOW_SELECTED, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Show Selected Groups"},
        {3, IDB_HIDE_SELECTED, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Hide Selected Groups"},
        {4, IDB_CLEAR_SELECTED, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Clear Selected Groups"},
        {10, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, -1},
        {5, IDB_SHOW_ALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Show All Groups"},
        {6, IDB_CLEAR_ALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Clear All Groups"},
        {10, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, -1},
        {7, IDB_GROUP, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Select Pre-Filter Groups"},
        {8, IDB_ELEMENT, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Select Pre-Filter Elements"},
        {10, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, -1},
        {9, IDB_RELOAD, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Reload Groups from Model"},
        {10, IDB_HIGHLIGHT, TBSTATE_ENABLED, TBSTYLE_DROPDOWN, {0}, 0, L"Highlight Selected Groups"},
    };

    SendMessage(hwnd_toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hwnd_toolbar, TB_ADDBUTTONS, (WPARAM)numButtons+4, (LPARAM)&tbButtons);

    return hwnd_toolbar;
}

LRESULT CALLBACK WndProc_toolbar(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (message)
    {
    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)(wParam);
        RECT rc; GetClientRect(hWnd, &rc);
        HBRUSH brush = CreateSolidBrush(RGB(188, 199, 216));
        FillRect(hdc, &rc, brush);
        DeleteObject(brush);
        return TRUE;
    }
    }
    return DefSubclassProc(hWnd, message, wParam, lParam);
}