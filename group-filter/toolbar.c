#include <Windows.h>
#include <commctrl.h>
#include "Resource.h"
#include "femap.h"

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
Reload
Highlight
*/

static LRESULT CALLBACK WndProc_toolbar(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
static void menu_displayoptions(HINSTANCE hInst, HWND hWnd, LPNMTOOLBAR lpnmTB, void* model);
static void menu_highlight(HINSTANCE hInst, HWND hWnd, LPNMTOOLBAR lpnmTB);

HWND toolbar_create(HWND hwnd_parent, HINSTANCE hInstance)
{
    const int ImageListID = 0;
    const int numButtons  = 11;
    const int bitmapSize  = 16;

    HWND hwnd_toolbar = CreateWindowEx(
        0,
        TOOLBARCLASSNAME,
        NULL,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
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
        {0, IDM_DISPLAY_OPTIONS, TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN, {0}, 0, L""},
        {10, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, -1},
        {1, IDM_SHOW_SELECTED_ONLY, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Show Selected Groups Only"},
        {2, IDM_SHOW_SELECTED, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Show Selected Groups"},
        {3, IDM_HIDE_SELECTED, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Hide Selected Groups"},
        {4, IDM_CLEAR_SELECTED, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Clear Selected Groups"},
        {10, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, -1},
        {5, IDM_SHOW_ALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Show All Groups"},
        {6, IDM_CLEAR_ALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Clear All Groups"},
        {10, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, -1},
        {7, IDM_GROUP, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Select Pre-Filter Groups"},
        {8, IDM_ELEMENT, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Select Pre-Filter Elements"},
        {10, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, -1},
        {9, IDM_RELOAD, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, L"Reload Groups from Model"},
        {10, IDM_HIGHLIGHT_OPTIONS, TBSTATE_ENABLED, TBSTYLE_DROPDOWN | BTNS_CHECK, {0}, 0, L"Highlight Selected Groups"},
    };

    SendMessage(hwnd_toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hwnd_toolbar, TB_ADDBUTTONS, (WPARAM)numButtons+4, (LPARAM)&tbButtons);

    return hwnd_toolbar;
}

void toolbar_notify(HINSTANCE hInst, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, void *model)
{
    LPNMHDR lpnm = ((LPNMHDR)lParam);
    LPNMTOOLBAR lpnmTB = ((LPNMTOOLBAR)lParam);
    switch (lpnm->code)
    {
        case TBN_DROPDOWN:
        {
            switch (lpnmTB->iItem)
            {
            case IDM_DISPLAY_OPTIONS:
                menu_displayoptions(hInst, hWnd, lpnmTB, model);
                break;
            case IDM_HIGHLIGHT_OPTIONS:
                menu_highlight(hInst, hWnd, lpnmTB);
                break;
            }
        }
    }
}

static void menu_displayoptions(HINSTANCE hInst, HWND hWnd, LPNMTOOLBAR lpnmTB, void* model) {
    HMENU hMenuLoaded = LoadMenu(hInst, MAKEINTRESOURCE(IDR_DISPLAY_OPTIONS));

    RECT rc;
    SendMessage(lpnmTB->hdr.hwndFrom, TB_GETRECT, (WPARAM)lpnmTB->iItem, (LPARAM)&rc);
    MapWindowPoints(lpnmTB->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&rc, 2);

    HMENU hPopupMenu = GetSubMenu(hMenuLoaded, 0);
    CheckMenuRadioItem(hPopupMenu, 0, 2, 0, MF_BYPOSITION);

    TPMPARAMS tpm;
    tpm.cbSize = sizeof(TPMPARAMS);
    tpm.rcExclude = rc;
    TrackPopupMenuEx(hPopupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, hWnd, &tpm);

    DestroyMenu(hMenuLoaded);
}

static void menu_highlight(HINSTANCE hInst, HWND hWnd, LPNMTOOLBAR lpnmTB) {
    HMENU hMenuLoaded = LoadMenu(hInst, MAKEINTRESOURCE(IDR_HIGHLIGHT_OPTIONS));
    // Get the coordinates of the button.
    RECT rc;
    SendMessage(lpnmTB->hdr.hwndFrom, TB_GETRECT, (WPARAM)lpnmTB->iItem, (LPARAM)&rc);
    MapWindowPoints(lpnmTB->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&rc, 2);

    HMENU hPopupMenu = GetSubMenu(hMenuLoaded, 0);
    TPMPARAMS tpm;
    tpm.cbSize = sizeof(TPMPARAMS);
    tpm.rcExclude = rc;
    TrackPopupMenuEx(hPopupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, hWnd, &tpm);
    DestroyMenu(hMenuLoaded);
}

// Info_ViewShowErase
// feAppModelInfoShow
// feAppSetModelInfoShow


static LRESULT CALLBACK WndProc_toolbar(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
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