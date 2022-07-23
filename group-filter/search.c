#include <Windows.h>
#include <commctrl.h>
#include "Resource.h"
#include "search.h"

HWND search_create(HWND hwnd_parent, HINSTANCE hInstance)
{
    RECT cr;
    GetClientRect(hwnd_parent, &cr);

    HWND hwnd_groupbox = CreateWindowEx(
        0,
        L"BUTTON",
        NULL,
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        7, 27, cr.right - cr.left - 14, 0,
        hwnd_parent, NULL, hInstance, NULL
    );

    HWND hwnd_combobox = CreateWindowEx(
        0,
        L"COMBOBOX",
        L"",
        CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
        7, 35, 55, 33,
        hwnd_parent, NULL, hInstance, NULL
    );

    WCHAR A[2][20] = { L"Title", L"ID" };
    SendMessage(hwnd_combobox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A[0]);
    SendMessage(hwnd_combobox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A[1]);
    SendMessage(hwnd_combobox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

    HWND hwnd_search = CreateWindowEx(
        0,
        L"EDIT",
        L"Filter by Title",
        WS_CHILD | WS_VISIBLE,
        70, 40, 150, 18,
        hwnd_parent, NULL, hInstance, NULL
    );

    HFONT hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, TEXT("Segoe UI"));
    SendMessage(hwnd_search, WM_SETFONT, (WPARAM)hFont, TRUE);


    return hwnd_groupbox;
}
