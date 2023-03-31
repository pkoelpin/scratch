
/* This is so the dll will use common controls V6 */
#define ISOLATION_AWARE_ENABLED 1

/* Use the unicode functions in win32 */
#define UNICODE

/* The pragma supresses warnings from the standard headers */
#pragma warning(push, 0)
#include <windows.h>
#include <commctrl.h>
#include <stdbool.h>
#pragma warning(pop)

#include "resource.h"
#include "condlist.h"

#define CLASSNAME "SelectCondition"
#define DLLNAME "fes"

struct state {
    condlist *left;
    condlist *right;
};

struct state* state_create(int *id, const char * const *title, int n) {
    struct state *s = (struct state*)HeapAlloc(GetProcessHeap(), 0, sizeof(struct state));
    s->left = condlist_create(id, title, n, false);
    s->right = condlist_create(id, title, n, true);
    return s;
}

void state_free(struct state *s){
    condlist_free(s->left);
    condlist_free(s->right);
    HeapFree(GetProcessHeap(), 0, s);
}

/*
Convert an integer to a string. This function does absolute value so no negative
sign is included in the string.
REF: https://nullprogram.com/blog/2023/02/13/
*/
void append_long(unsigned char *buf, long x) {
    unsigned char tmp[128];
    unsigned char *end = tmp + sizeof(tmp);
    unsigned char *beg = end;
    long t = x>0 ? -x : x;
    do {
        *--beg = '\0';
        *--beg = '0' - t%10;
    } while (t /= 10);
    for (int i = 0; i < end-beg; i++) {
        buf[i] = beg[i];
    }
    buf[end-beg] = '\0';
    buf[end-beg+1] = '\0';
}

void update(HWND hwnd) {
    struct state *s = (struct state*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    /* Get the text in the search bars */
    HWND hwnd_search_left = GetDlgItem(hwnd, ID_SEARCH_LEFT);
    HWND hwnd_search_right = GetDlgItem(hwnd, ID_SEARCH_RIGHT);

    LPTSTR text_left_wide[1024];
    LPTSTR text_right_wide[1024];

    GetWindowText(hwnd_search_left, (LPTSTR)text_left_wide, 1024);
    GetWindowText(hwnd_search_right, (LPTSTR)text_right_wide, 1024);

    char text_left[1024];
    char text_right[1024];

    WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)text_left_wide, 1024, text_left, 1024, NULL, NULL);
    WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)text_right_wide, 1024, text_right, 1024, NULL, NULL);

    condlist_update(s->left, text_left);
    condlist_update(s->right, text_right);

    HWND hwnd_left = GetDlgItem(hwnd, ID_LISTVIEW_LEFT);
    HWND hwnd_right = GetDlgItem(hwnd, ID_LISTVIEW_RIGHT);

    ListView_SetItemCount(hwnd_left, condlist_len(s->left));
    ListView_SetItemCount(hwnd_right, condlist_len(s->right));
}

/* 
Generic function to create all the buttons for the window
*/
void button_create(HWND hwnd_parent, HINSTANCE hInst, LPCTSTR text, long long id) {
    CreateWindowEx(
        0,
        TEXT("BUTTON"),
        text,
        WS_VISIBLE | WS_CHILD | WS_TABSTOP,
        0, 0, 0, 0,
        hwnd_parent, (HMENU)id, hInst, NULL
    );
}

/*
This function sets up the listview controls 
*/
void listview_create(HWND hwnd_parent, HINSTANCE hInst, long long id) {
    HWND hwnd = CreateWindowEx(
        0,
        WC_LISTVIEW,
        NULL,
        WS_CHILD | LVS_REPORT | WS_VISIBLE | LVS_OWNERDATA | WS_BORDER | LVS_SHOWSELALWAYS,
        0, 0, 0, 0,
        hwnd_parent, (HMENU)id, hInst, NULL
    );

    ListView_SetExtendedListViewStyleEx(hwnd, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

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

void search_create(HWND hwnd_parent, HINSTANCE hInst, long long id) {
    HWND hwnd = CreateWindowEx(
        0,
        TEXT("EDIT"),
        TEXT(""),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        0, 0, 0, 0,
        hwnd_parent, (HMENU)id, hInst, NULL
    );

    /* This function only comes in UTF-16, so that's why it's L"Search" */
    Edit_SetCueBannerText(hwnd, L"Search");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    struct state *s = (struct state*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg) {
    case WM_CREATE: {
        HINSTANCE hInst = ((CREATESTRUCT*)lParam)->hInstance;
        listview_create(hwnd, hInst, ID_LISTVIEW_LEFT);
        listview_create(hwnd, hInst, ID_LISTVIEW_RIGHT);
        search_create(hwnd, hInst, ID_SEARCH_LEFT);
        search_create(hwnd, hInst, ID_SEARCH_RIGHT);
        button_create(hwnd, hInst, TEXT(">>"), ID_BUTTON_PICK_ALL);
        button_create(hwnd, hInst, TEXT(">"), ID_BUTTON_PICK_SELECTED);
        button_create(hwnd, hInst, TEXT("<"), ID_BUTTON_UNPICK_SELECTED);
        button_create(hwnd, hInst, TEXT("<<"), ID_BUTTON_UNPICK_ALL);
        button_create(hwnd, hInst, TEXT("Undo"), ID_BUTTON_UNDO);
        button_create(hwnd, hInst, TEXT("Redo"), ID_BUTTON_REDO);
        button_create(hwnd, hInst, TEXT("OK"), ID_BUTTON_OK);
        } return 0;
    case WM_DESTROY:{
        PostQuitMessage(0);
        } return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
        EndPaint(hwnd, &ps);
        } return 0;
    case WM_SIZE: {
        UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
        int WINDOW_H = HIWORD(lParam);
        int WINDOW_W = LOWORD(lParam);
        int GUTTER = 12;
        int PADDING = 8;
        int BUTTON_W = 120;
        int BUTTON_H = 32;

        /* Position the left search bar */
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
        } return 0;
    case WM_NOTIFY: {
        LPNMHDR lpnmhdr = (LPNMHDR)lParam;
        switch (lpnmhdr->idFrom) {
        case ID_LISTVIEW_LEFT:
        case ID_LISTVIEW_RIGHT:
            condlist *list = lpnmhdr->idFrom == ID_LISTVIEW_LEFT ? s->left : s->right;
            switch (lpnmhdr->code) {
            case LVN_GETDISPINFO:
                NMLVDISPINFO* lpdi = (NMLVDISPINFO*)lParam;
                {
                    switch(lpdi->item.iSubItem) {
                    case 0:
                        if (lpdi->item.mask & LVIF_TEXT) {
                            append_long((unsigned char *)lpdi->item.pszText, condlist_get_id(list, lpdi->item.iItem));
                        }
                        break;
                    case 1:
                        MultiByteToWideChar(CP_UTF8, 0, condlist_get_title(list, lpdi->item.iItem), -1, lpdi->item.pszText, lpdi->item.cchTextMax);
                        break;
                    }
                }
            }
            break;
        }
        }return 0;
    case WM_COMMAND:{
        switch (LOWORD(wParam)){
            case ID_BUTTON_PICK_ALL:
                if (HIWORD(wParam) == BN_CLICKED) {
                    for (int i = 0; i < condlist_len(s->left); i++) {
                        condlist_flip(s->left, i);
                    }
                }
                ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_LEFT), -1, 0, LVIS_SELECTED);
                ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT), -1, 0, LVIS_SELECTED);
                update(hwnd);
                break;
            case ID_BUTTON_UNPICK_ALL:
                if (HIWORD(wParam) == BN_CLICKED) {
                    for (int i = 0; i < condlist_len(s->right); i++) {
                        condlist_flip(s->right, i);
                    }
                }
                ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_LEFT), -1, 0, LVIS_SELECTED);
                ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT), -1, 0, LVIS_SELECTED);
                update(hwnd);
                break;
            case ID_BUTTON_PICK_SELECTED:
                if (HIWORD(wParam) == BN_CLICKED) {
                    int count = ListView_GetSelectedCount(GetDlgItem(hwnd, ID_LISTVIEW_LEFT));
                    int index = -1;
                    for (int i = 0; i < count; i++) {
                        index = ListView_GetNextItem(GetDlgItem(hwnd, ID_LISTVIEW_LEFT), index, LVNI_SELECTED);
                        condlist_flip(s->left, index);
                    }
                }
                ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_LEFT), -1, 0, LVIS_SELECTED);
                ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT), -1, 0, LVIS_SELECTED);
                update(hwnd);
                break;
            case ID_BUTTON_UNPICK_SELECTED:
                if (HIWORD(wParam) == BN_CLICKED) {
                    int count = ListView_GetSelectedCount(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT));
                    int index = -1;
                    for (int i = 0; i < count; i++) {
                        index = ListView_GetNextItem(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT), index, LVNI_SELECTED);
                        condlist_flip(s->right, index);
                    }
                }
                ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_LEFT), -1, 0, LVIS_SELECTED);
                ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT), -1, 0, LVIS_SELECTED);
                update(hwnd);
                break;
            case ID_SELECTALL:
                ListView_SetItemState(GetFocus(), -1, LVIS_SELECTED, LVIS_SELECTED);
                break;
            case ID_SEARCH_LEFT:
            case ID_SEARCH_RIGHT:
                if (HIWORD(wParam) == EN_CHANGE) {
                    update(hwnd);
                }
                break;
        }
    }
    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int _DllMainCRTStartup(void) {
    return 1;
}

__declspec(dllexport)
int fes_cond(int *id, const char * const *title, int n) {
    HINSTANCE hinstance = GetModuleHandle(TEXT(DLLNAME));

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
    wc.hIcon         = NULL;
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

    /* Set up the struct that holds the state info for the condition list */
    struct state *s = state_create(id, title, n);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)s);

    ShowWindow(hwnd, SW_SHOW);
    update(hwnd);

    HACCEL hAccelTable = LoadAccelerators(hinstance, MAKEINTRESOURCE(ID_ACCEL));

    MSG message;
    while(GetMessage(&message, NULL, 0, 0))
    {

        if (!TranslateAccelerator(hwnd, hAccelTable, &message)){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
    
    state_free(s);

    return 0;
}

