#include <Windows.h>
#include <commctrl.h>
#include "Resource.h"
#include "search.h"
#include "entitylist.h"

HWND search_create(HWND hwnd_parent, HINSTANCE hInstance)
{
    RECT cr;
    GetClientRect(hwnd_parent, &cr);

    HWND hwnd_search = CreateWindowEx(
        0,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        0, 0, 0, 0,
        hwnd_parent, IDM_SEARCHBAR, hInstance, NULL
    );

    Edit_SetCueBannerText(hwnd_search, L"Search Groups");

    return hwnd_search;
}

void search_command(HINSTANCE hInst, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, entitylist* el)
{
    HWND hwnd_search = (HWND)lParam;
    if (HIWORD(wParam) == EN_CHANGE)
    {
        wchar_t search_string[256];
        GetWindowText(hwnd_search, search_string+1, 253);
        search_string[0] = '*';
        int len = wcslen(search_string);
        search_string[len] = '*';
        search_string[len+1] = '\0';
        entitylist_filter(el, search_string);
    }
}