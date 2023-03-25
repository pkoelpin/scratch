// Raymond Chen Icon
//https://devblogs.microsoft.com/oldnewthing/20101018-00/?p=12513

// Icon generator
// https://www.icoconverter.com/
// favicon.io

// Design Guide
// https://learn.microsoft.com/en-us/windows/apps/design/

// Unicode stuff
// https://learn.microsoft.com/en-us/windows/apps/design/globalizing/use-utf8-code-page

// Embed manifest
// https://learn.microsoft.com/en-us/cpp/build/understanding-manifest-generation-for-c-cpp-programs?view=msvc-170

#ifdef _MSC_VER
    #pragma comment(linker, "/subsystem:windows")
    #pragma comment(lib, "kernel32.lib")
    #pragma comment(lib, "user32.lib")
    #pragma comment(lib, "comctl32.lib")
#endif

#define CLASSNAME "SelectCondition"

#include <windows.h>
#include <commctrl.h>
#include <stdbool.h>
#include "resource.h"

#pragma function(memcpy)
void *memcpy(unsigned char *d, unsigned const char *s, size_t n) {
    __movsb(d, s, n);
    return d;
}

struct clist *left;
struct clist *right;

struct clist {
    bool ispicked;
    int n;
    int *id;
    const char **title;
    int count;
    int *idx;
};

#define CLIST_GET_ID(i) 

void clist_refresh(struct clist *list) {
    list->count = 0;
    for (int i=0; i<list->n; i++) {
        if ((list->id[i] > 0) == list->ispicked) {
            list->idx[list->count++] = i;
        }
    }
}

struct clist* clist_init(int n, int *id, const char  **title, bool ispicked) {
    struct clist *list = (struct clist*)HeapAlloc(GetProcessHeap(), 0, sizeof(struct clist));
    list->ispicked = ispicked;
    list->n = n;
    list->id = id;
    list->title = title;
    list->count = 0;
    list->idx = (int*)HeapAlloc(GetProcessHeap(), 0, n*sizeof(int));
    clist_refresh(list);
    return list;
}

void append_long(unsigned char *buf, long x)
{
    unsigned char tmp[64];
    unsigned char *end = tmp + sizeof(tmp);
    unsigned char *beg = end;
    long t = x>0 ? -x : x;
    do {
        *--beg = '0' - t%10;
    } while (t /= 10);
    for (int i = 0; i < end-beg; i++) {
        buf[i] = beg[i];
    }
    buf[end-beg] = '\0';
}

void button_create(HWND hwnd_parent, HINSTANCE hInstance, LPCTSTR text, long long id) {
    CreateWindowEx(
        0,
        TEXT("BUTTON"),
        text,
        WS_VISIBLE | WS_CHILD | WS_TABSTOP,
        0, 0, 0, 0,
        hwnd_parent, (HMENU)id, hInstance, NULL
    );
}

void update(HWND hwnd) {
    clist_refresh(left); 
    clist_refresh(right); 

    HWND hwnd_left = GetDlgItem(hwnd, ID_LISTVIEW_LEFT);
    HWND hwnd_right = GetDlgItem(hwnd, ID_LISTVIEW_RIGHT);

    // Set the size
    ListView_SetItemCount(hwnd_left, left->count);
    ListView_SetItemCount(hwnd_right, right->count);

}

void listview_create(HWND hwnd_parent, HINSTANCE hInstance, long long id) {
    HWND hwnd = CreateWindowEx(
        0,
        WC_LISTVIEW,
        NULL,
        WS_CHILD | LVS_REPORT | WS_VISIBLE | LVS_OWNERDATA | WS_BORDER,
        0, 0, 0, 0,
        hwnd_parent, (HMENU)id, hInstance, NULL
    );

    ListView_SetExtendedListViewStyleEx(hwnd, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    // Setup the columns
    LV_COLUMN lvC;
    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvC.fmt = LVCFMT_LEFT;
    lvC.cchTextMax = 80;
    lvC.cx = 50;
    lvC.pszText = TEXT("ID");
    ListView_InsertColumn(hwnd, 0, &lvC);

    lvC.pszText = TEXT("Title");
    lvC.fmt = LVCFMT_LEFT;
    lvC.cx = 200;
    ListView_InsertColumn(hwnd, 1, &lvC); 
}

void search_create(HWND hwnd_parent, HINSTANCE hInstance, long long id) {
    HWND hwnd = CreateWindowEx(
        0,
        TEXT("EDIT"),
        TEXT(""),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        0, 0, 0, 0,
        hwnd_parent, (HMENU)id, hInstance, NULL
    );
    Edit_SetCueBannerText(hwnd, L"Search");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    switch (uMsg)
    {
    case WM_CREATE:
        {
            HINSTANCE hinst = ((CREATESTRUCT*)lParam)->hInstance;
            listview_create(hwnd, hinst, ID_LISTVIEW_LEFT);
            listview_create(hwnd, hinst, ID_LISTVIEW_RIGHT);
            search_create(hwnd, hinst, ID_SEARCH_LEFT);
            search_create(hwnd, hinst, ID_SEARCH_RIGHT);
            button_create(hwnd, hinst, TEXT(">>"), ID_BUTTON_PICK_ALL);
            button_create(hwnd, hinst, TEXT(">"), ID_BUTTON_PICK_SELECTED);
            button_create(hwnd, hinst, TEXT("<"), ID_BUTTON_UNPICK_SELECTED);
            button_create(hwnd, hinst, TEXT("<<"), ID_BUTTON_UNPICK_ALL);
            button_create(hwnd, hinst, TEXT("Undo"), ID_BUTTON_UNDO);
            button_create(hwnd, hinst, TEXT("Redo"), ID_BUTTON_REDO);
            button_create(hwnd, hinst, TEXT("OK"), ID_BUTTON_OK);
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
            EndPaint(hwnd, &ps);
        }
        return 0;
    case WM_SIZE:
        {
            UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
            int WINDOW_H = HIWORD(lParam);
            int WINDOW_W = LOWORD(lParam);
            int GUTTER = 12;
            int PADDING = 8;
            int BUTTON_W = 120;
            int BUTTON_H = 32;

            // Position the left search bar
            int SL_X = GUTTER;
            int SL_Y = GUTTER;
            int SL_H = 23;
            int SL_W = WINDOW_W/2 - GUTTER - PADDING - BUTTON_W/2;
            SetWindowPos(GetDlgItem(hwnd, ID_SEARCH_LEFT), 0, 
                SL_X, SL_Y, SL_W, SL_H, flags);

            // Position of right search bar
            int SR_X = SL_X + SL_W + BUTTON_W + PADDING*2;
            int SR_Y = SL_Y;
            int SR_H = SL_H;
            int SR_W = SL_W;
            SetWindowPos(GetDlgItem(hwnd, ID_SEARCH_RIGHT), 0, 
                SR_X, SR_Y, SR_W, SR_H, flags);

            // Position the left listview
            int LVL_X = GUTTER;
            int LISTVIEW_Y = SL_Y + SL_H + PADDING;
            int LISTVIEW_H = WINDOW_H - LISTVIEW_Y - GUTTER;
            int LISTVIEW_W = SL_W;
            SetWindowPos(GetDlgItem(hwnd, ID_LISTVIEW_LEFT), 0, 
                LVL_X, LISTVIEW_Y, LISTVIEW_W, LISTVIEW_H, flags);

            // Position the right listview
            int LVR_X = SR_X;
            SetWindowPos(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT), 0, 
                LVR_X, LISTVIEW_Y, LISTVIEW_W, LISTVIEW_H, flags);

            // Button Positioning
            int BUTTON_X = SL_X + SL_W + PADDING;

            int B_PICK_ALL_Y = LISTVIEW_Y + LISTVIEW_H/2 - BUTTON_H*3.5 - PADDING*3;
            SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_PICK_ALL), 0, 
                BUTTON_X, B_PICK_ALL_Y, BUTTON_W, BUTTON_H, flags);

            int B_PICK_SELECTED_Y = LISTVIEW_Y + LISTVIEW_H/2 - BUTTON_H*2.5 - PADDING*2;
            SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_PICK_SELECTED), 0,
                BUTTON_X, B_PICK_SELECTED_Y, BUTTON_W, BUTTON_H, flags);

            int B_UNPICK_SELECTED_Y = LISTVIEW_Y + LISTVIEW_H/2 - BUTTON_H*1.5 - PADDING;
            SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_UNPICK_SELECTED), 0, 
                BUTTON_X, B_UNPICK_SELECTED_Y, BUTTON_W, BUTTON_H, flags);

            int B_UNPICK_ALL_Y = LISTVIEW_Y + LISTVIEW_H/2 - BUTTON_H*0.5;
            SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_UNPICK_ALL), 0, 
                BUTTON_X, B_UNPICK_ALL_Y, BUTTON_W, BUTTON_H, flags);

            int B_UNDO_Y = LISTVIEW_Y + LISTVIEW_H/2 + BUTTON_H*0.5 + PADDING;
            SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_UNDO), 0, 
                BUTTON_X, B_UNDO_Y, BUTTON_W, BUTTON_H, flags);

            int B_REDO_Y = LISTVIEW_Y + LISTVIEW_H/2 + BUTTON_H*1.5 + PADDING*2;
            SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_REDO), 0, 
                BUTTON_X, B_REDO_Y, BUTTON_W, BUTTON_H, flags);

            int B_OK_Y = LISTVIEW_Y + LISTVIEW_H/2 + BUTTON_H*2.5 + PADDING*3;
            SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_OK), 0, 
                BUTTON_X, B_OK_Y, BUTTON_W, BUTTON_H, flags);
        }
        return 0;
    case WM_NOTIFY:
        LPNMHDR lpnmhdr = (LPNMHDR)lParam;
        switch (lpnmhdr->idFrom )
        {
        case ID_LISTVIEW_LEFT:
        case ID_LISTVIEW_RIGHT:
            struct clist *list = lpnmhdr->idFrom == ID_LISTVIEW_LEFT ? left : right;
            switch (lpnmhdr->code) 
            {
            case LVN_GETDISPINFO:
                NMLVDISPINFO* lpdi = (NMLVDISPINFO*)lParam;
                {
                    switch(lpdi->item.iSubItem)
                    {
                    case 0:
                        if (lpdi->item.mask & LVIF_TEXT) {
                            append_long(lpdi->item.pszText, list->id[list->idx[lpdi->item.iItem]]);
                        }
                        break;
                    case 1:
                        lpdi->item.pszText = list->title[list->idx[lpdi->item.iItem]];
                        break;
                    }
                }
            }
            break;
        }
        return 0;
    case WM_COMMAND:
        {
            if ((LOWORD(wParam) == ID_BUTTON_PICK_ALL) & (HIWORD(wParam) == BN_CLICKED)) {
                for (int i = 0; i < left->count; i++){
                    left->id[left->idx[i]] = -left->id[left->idx[i]];
                }
                update(hwnd);
            }
        }
        return 0;

    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int ktk_select_cond(int n, int *id, const char **title) 
{

    left = clist_init(n, id, title, false);
    right = clist_init(n, id, title, true);

    HINSTANCE hinstance = GetModuleHandle(NULL);

    INITCOMMONCONTROLSEX icex;
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEX wc;
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hinstance;
    wc.hIcon         = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_KTK_SELECTCOND));
    wc.hCursor       = NULL;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = TEXT(CLASSNAME);
    wc.hIconSm       = NULL;

    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        TEXT(CLASSNAME),
        TEXT("Select Active Conditions"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 500,
        NULL, NULL, hinstance, NULL
    );

    ShowWindow(hwnd, SW_SHOW);

    update(hwnd);

    MSG message;
    while(GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return 0;
}


int WinMainCRTStartup(void)
{
    int n = 10;
    int id[10] = {-1, 2, 3, -4, 5, 6, -7, 8, 9, 10};
    char *title[10];
    for (int i = 0; i < n; i++) {
        title[i] = "Subcase";
    }
    ktk_select_cond(n, id, title);

    ExitProcess(0);
}