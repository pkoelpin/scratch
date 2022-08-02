#include <Windows.h>
#include <commctrl.h>
#include "Resource.h"
#include "femap.h"

/*
Select type
    Groups
    Properties
    Materials
    Layups
Reload Model
Reset All Visibility Options
Group Select
Element Select
Show When Selected (Highlight)


Highlight functions
    Info_ViewShowErase
    feAppModelInfoShow
    feAppSetModelInfoShow
*/

static void menu_displayoptions(HINSTANCE hInst, HWND hWnd, LPNMTOOLBAR lpnmTB, void* model);
static void menu_highlight(HINSTANCE hInst, HWND hWnd, LPNMTOOLBAR lpnmTB);

HWND toolbar_create(HWND hwnd_parent, HINSTANCE hInstance)
{
    const int ImageListID = 0;
    const int numButtons  = 6;
    const int bitmapSize  = 16;

    HWND hwnd_toolbar = CreateWindowEx(
        0,
        TOOLBARCLASSNAME,
        NULL,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
        0, 0, 0, 0,
        hwnd_parent, ID_TOOLBAR, hInstance, NULL);

    if (hwnd_toolbar == NULL)
        return NULL;

    /* Give the toolbar dropdown arrows */
    SendMessage(hwnd_toolbar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);

    /* Set the toolbar so that only buttons are display; no text */
    SendMessage(hwnd_toolbar, TB_SETMAXTEXTROWS, 0, 0);

    /* Setup the image list */
    HIMAGELIST hImageList = ImageList_Create(
        bitmapSize, 
        bitmapSize,  
        ILC_COLOR24 | ILC_MASK,
        numButtons,
        0);

    SendMessage(
        hwnd_toolbar, 
        TB_SETIMAGELIST,
        (WPARAM)ImageListID,
        (LPARAM)hImageList);

    /* Add the bitmaps to the image lists */
    int bitmaps[] = { 
        IDB_GROUP,
        IDB_PROPERTY, 
        IDB_MATERIAL, 
        IDB_LAYUP,
        IDB_RELOAD,
        IDB_RESET_VISIBILITY,
        IDB_GROUP,
        IDB_ELEMENT,
        IDB_HIGHLIGHT
    };

    for (int i = 0; i < 9; i++) {
        HBITMAP hbmp = LoadBitmap(hInstance, MAKEINTRESOURCE(bitmaps[i]));
        ImageList_AddMasked(hImageList, hbmp, RGB(192, 192, 192));
        DeleteObject(hbmp);
    }

    /* Setup the buttons for the toolbar */
    TBBUTTON tbButtons[] = 
    {
        {0, IDM_DISPLAY_OPTIONS, TBSTATE_INDETERMINATE, BTNS_WHOLEDROPDOWN, {0}, 0, L"Select Entity to Filter"},
        {4, IDM_RELOAD, TBSTATE_INDETERMINATE, TBSTYLE_BUTTON, {0}, 0, L"Reload Model"},
        {5, IDM_SHOW_SELECTED, TBSTATE_INDETERMINATE, TBSTYLE_BUTTON, {0}, 0, L"Reset All Visibility Options"},
        {6, IDM_GROUP, TBSTATE_INDETERMINATE, TBSTYLE_DROPDOWN | BTNS_CHECK, {0}, 0, L"Group Select"},
        {7, IDM_ELEMENT, TBSTATE_INDETERMINATE, TBSTYLE_DROPDOWN | BTNS_CHECK, {0}, 0, L"Element Select"},
        {8, IDM_HIGHLIGHT_OPTIONS, TBSTATE_INDETERMINATE, TBSTYLE_DROPDOWN | BTNS_CHECK, {0}, 0, L"Highlight Selected Groups"},
    };

    /* Add the buttons to the toolbar */
    SendMessage(hwnd_toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hwnd_toolbar, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)&tbButtons);

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

/* Popup menu when user selects the highlight button */
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