/* https://www.icoconverter.com/ */

/* This is so the dll will use common controls V6 */
#define ISOLATION_AWARE_ENABLED 1

/* Use the unicode functions in win32 */
#define UNICODE

/* The pragma supresses warnings from the standard headers */
#include <windows.h>
#include <commctrl.h>
#include <stdbool.h>
#include "resource.h"
#include "condlist.h"
#include "strbuf.h"

#include "stdlib.h"
#include "search.h"
#include "stdio.h"

#define CLASSNAME "SelectCondition"
#define DLLNAME "fepick"

static wchar_t msgbuf[1024]; 

struct state {
    int n;
    condlist *left;
    condlist *right;
    int *buf;
    int buf_cur;
    int buf_used;
    int buf_len;
    int *undo_count_list;
    bool ok;
    HINSTANCE hInst; 
};

struct state* state_create(int *id, const char * const *title, int n, HINSTANCE hInst) {
    struct state *s = (struct state*)HeapAlloc(GetProcessHeap(), 0, sizeof(struct state));
    s->n = n;
    s->buf = (int*)HeapAlloc(GetProcessHeap(), 0, n*sizeof(int));
    s->undo_count_list = (int*)HeapAlloc(GetProcessHeap(), 0, sizeof(int));
    for (int i=0; i<n; i++){
        s->buf[i] = id[i];
    }
    s->buf_cur = 0;
    s->buf_used = 1;
    s->buf_len = 1;
    s->left = condlist_create(s->buf, title, n, false);
    s->right = condlist_create(s->buf, title, n, true);
    s->ok = false;
    s->hInst = hInst;
    return s;
}

void state_advance(struct state *s) {
    if (s->buf_used == s->buf_len) {
        s->buf_len *= 2;
        s->buf = HeapReAlloc(GetProcessHeap(), 0, s->buf, s->n * s->buf_len * sizeof(int));
        s->undo_count_list = HeapReAlloc(GetProcessHeap(), 0, s->undo_count_list, s->buf_len * sizeof(int));
    }
    for (int i=0; i<s->n; i++) {
        s->buf[s->n*(s->buf_cur+1) + i] = s->buf[s->n*s->buf_cur + i];
    }
    s->undo_count_list[s->buf_cur] = condlist_count(s->right);
    s->buf_cur++;
    condlist_set_id(s->left, s->buf + s->n*s->buf_cur);
    condlist_set_id(s->right, s->buf + s->n*s->buf_cur);
    s->buf_used = s->buf_cur + 1;
}

void state_undo(struct state *s) {
    if (s->buf_cur == 0) {
        return;
    }
    s->undo_count_list[s->buf_cur] = condlist_count(s->right);
    s->buf_cur--;
    condlist_set_id(s->left, s->buf + s->n*s->buf_cur);
    condlist_set_id(s->right, s->buf + s->n*s->buf_cur);
}

void state_redo(struct state *s) {
    if (s->buf_cur == s->buf_used - 1) {
        return;
    }
    s->buf_cur++;
    condlist_set_id(s->left, s->buf + s->n*s->buf_cur);
    condlist_set_id(s->right, s->buf + s->n*s->buf_cur);
}

void state_free(struct state *s){
    condlist_free(s->left);
    condlist_free(s->right);
    HeapFree(GetProcessHeap(), 0, s->buf);
    HeapFree(GetProcessHeap(), 0, s);
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

    /* update the lists with the text in the search bars*/
    condlist_update(s->left, text_left);
    condlist_update(s->right, text_right);

    /* give the listviews a new item count */
    HWND hwnd_left = GetDlgItem(hwnd, ID_LISTVIEW_LEFT);
    HWND hwnd_right = GetDlgItem(hwnd, ID_LISTVIEW_RIGHT);
    ListView_SetItemCount(hwnd_left, condlist_len(s->left));
    ListView_SetItemCount(hwnd_right, condlist_len(s->right));

    /* uopate the status bar */
    struct strbuf b1 = MEMBUF(msgbuf, 256);
    APPEND_STR(&b1, L"Inactive: ");
    append_long(&b1, condlist_count(s->left));
    APPEND_STR(&b1, L"\0");
    SendMessage(GetDlgItem(hwnd, ID_STATUSBAR), SB_SETTEXT, 0, (LPARAM)msgbuf);

    struct strbuf b2 = MEMBUF(msgbuf+256, 256);
    APPEND_STR(&b2, L"Filtered: ");
    append_long(&b2, condlist_len(s->left));
    APPEND_STR(&b2, L"\0");
    SendMessage(GetDlgItem(hwnd, ID_STATUSBAR), SB_SETTEXT, 1, (LPARAM)(msgbuf+256));

    struct strbuf b3 = MEMBUF(msgbuf+512, 256);
    APPEND_STR(&b3, L"Active: ");
    append_long(&b3, condlist_count(s->right));
    APPEND_STR(&b3, L"\0");
    SendMessage(GetDlgItem(hwnd, ID_STATUSBAR), SB_SETTEXT, 3, (LPARAM)(msgbuf+512));

    struct strbuf b4 = MEMBUF(msgbuf+768, 256);
    b1 = (struct strbuf) MEMBUF(msgbuf, 256);
    APPEND_STR(&b4, L"Filtered: ");
    append_long(&b4, condlist_len(s->right));
    APPEND_STR(&b4, L"\0");
    SendMessage(GetDlgItem(hwnd, ID_STATUSBAR), SB_SETTEXT, 4, (LPARAM)(msgbuf+768));

    /* enable/disable undo/redo */
    EnableWindow(GetDlgItem(hwnd, ID_BUTTON_UNDO), s->buf_cur != 0);
    EnableWindow(GetDlgItem(hwnd, ID_BUTTON_REDO), s->buf_cur != s->buf_used - 1);
}

/* Generic function to create all the buttons for the window */
void button_create(HWND hwnd_parent, HINSTANCE hInst, LPCTSTR text, long long id) {
    CreateWindowEx(
        0,
        TEXT("BUTTON"),
        text,
        WS_VISIBLE | WS_CHILD,
        0, 0, 0, 0,
        hwnd_parent, (HMENU)id, hInst, NULL
    );
}

void undo_redo_create(HWND hwnd_parent, HINSTANCE hInst, LPCTSTR text, long long id) {
    CreateWindowEx(
        0,
        TEXT("BUTTON"),
        text,
        WS_VISIBLE | WS_CHILD | BS_SPLITBUTTON,
        0, 0, 0, 0,
        hwnd_parent, (HMENU)id, hInst, NULL
    );
}

void listview_create(HWND hwnd_parent, HINSTANCE hInst, long long id) {
    HWND hwnd = CreateWindowEx(
        0,
        WC_LISTVIEW,
        NULL,
        WS_CHILD | LVS_REPORT | WS_VISIBLE | LVS_OWNERDATA  | LVS_SHOWSELALWAYS,
        0, 0, 0, 0,
        hwnd_parent, (HMENU)id, hInst, NULL
    );

    ListView_SetExtendedListViewStyleEx(hwnd, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    LVCOLUMN lvC;
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

void status_create(HWND hwnd_parent, HINSTANCE hInst, long long id) {
    HWND hwnd = CreateWindowEx(
        0,
        STATUSCLASSNAME,
        TEXT(""),
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hwnd_parent, (HMENU)id, hInst, NULL
    ); 
}

void groupbox_create(HWND hwnd_parent, HINSTANCE hInst, LPCTSTR text, long long id) {
    CreateWindowEx(
        0,
        TEXT("BUTTON"),
        text,
        WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
        0, 0, 0, 0,
        hwnd_parent, (HMENU)id, hInst, NULL
    );
}

void wm_size(HWND hwnd, int WINDOW_H, int WINDOW_W) {
    UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
    int GUTTER = 12;
    int PADDING = 8;
    int BUTTON_W = 120;
    int BUTTON_H = 32;
    int SEARCH_H = 23;
    int STATUS_H = 20;

    /* Get dimensions of status bar on the bottom */
    RECT sb_rect;
    SendMessage(GetDlgItem(hwnd, ID_STATUSBAR), SB_GETRECT , 0, (LPARAM)&sb_rect);
    int SB_H = sb_rect.bottom - sb_rect.top;

    /* groupbox positioning */
    int GBL_X = GUTTER;
    int GBL_Y = GUTTER;
    int GBL_W = WINDOW_W/2 - GUTTER - PADDING - BUTTON_W/2;
    int GBL_H = WINDOW_H - 2*GUTTER - SB_H;
    SetWindowPos(GetDlgItem(hwnd, ID_GROUPBOX_LEFT), 0, 
        GBL_X, GBL_Y, GBL_W, GBL_H, flags);

    int GBR_X = GBL_X + GBL_W + BUTTON_W + PADDING*2;
    int GBR_Y = GUTTER;
    int GBR_W = GBL_W;
    int GBR_H = GBL_H;
    SetWindowPos(GetDlgItem(hwnd, ID_GROUPBOX_RIGHT), 0, 
        GBR_X, GBR_Y, GBR_W, GBR_H, flags);

    /* Position search bars */
    int SL_X = GUTTER + PADDING;
    int SL_Y = GBL_Y + 3*PADDING;
    int SL_W = WINDOW_W/2 - GUTTER - PADDING - BUTTON_W/2 - 2*PADDING;
    int SL_H = SEARCH_H;
    SetWindowPos(GetDlgItem(hwnd, ID_SEARCH_LEFT), 0, 
        SL_X, SL_Y, SL_W, SL_H, flags);

    int SR_X = GBR_X + PADDING;
    int SR_Y = SL_Y;
    int SR_H = SL_H;
    int SR_W = SL_W;
    SetWindowPos(GetDlgItem(hwnd, ID_SEARCH_RIGHT), 0, 
        SR_X, SR_Y, SR_W, SR_H, flags);

    /* Position the listview */
    int LVL_X = SL_X;
    int LISTVIEW_Y = SL_Y + SL_H + PADDING;
    int LISTVIEW_W = SL_W;
    int LISTVIEW_H = GBL_H - SEARCH_H - PADDING*2 - (SL_Y - GBL_Y);
    SetWindowPos(GetDlgItem(hwnd, ID_LISTVIEW_LEFT), 0, 
        LVL_X, LISTVIEW_Y, LISTVIEW_W, LISTVIEW_H, flags);

    int LVR_X = GBR_X + PADDING;
    SetWindowPos(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT), 0, 
        LVR_X, LISTVIEW_Y, LISTVIEW_W, LISTVIEW_H, flags);

    /* Button Positioning */
    int BUTTON_X = GBL_X + GBL_W + PADDING;

    int B_PICK_ALL_Y = LISTVIEW_Y + LISTVIEW_H/2 - (int)(BUTTON_H*3.5) - PADDING*3;
    SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_PICK_ALL), 0, 
        BUTTON_X, B_PICK_ALL_Y, BUTTON_W, BUTTON_H, flags);

    int B_PICK_SELECTED_Y = LISTVIEW_Y + LISTVIEW_H/2 - (int)(BUTTON_H*2.5) - PADDING*2;
    SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_PICK_SELECTED), 0,
        BUTTON_X, B_PICK_SELECTED_Y, BUTTON_W, BUTTON_H, flags);

    int B_UNPICK_SELECTED_Y = LISTVIEW_Y + LISTVIEW_H/2 - (int)(BUTTON_H*1.5) - PADDING;
    SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_UNPICK_SELECTED), 0, 
        BUTTON_X, B_UNPICK_SELECTED_Y, BUTTON_W, BUTTON_H, flags);

    int B_UNPICK_ALL_Y = LISTVIEW_Y + LISTVIEW_H/2 - (int)(BUTTON_H*0.5);
    SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_UNPICK_ALL), 0, 
        BUTTON_X, B_UNPICK_ALL_Y, BUTTON_W, BUTTON_H, flags);

    int B_UNDO_Y = LISTVIEW_Y + LISTVIEW_H/2 + (int)(BUTTON_H*0.5) + PADDING;
    SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_UNDO), 0, 
        BUTTON_X, B_UNDO_Y, BUTTON_W, BUTTON_H, flags);

    int B_REDO_Y = LISTVIEW_Y + LISTVIEW_H/2 + (int)(BUTTON_H*1.5) + PADDING*2;
    SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_REDO), 0, 
        BUTTON_X, B_REDO_Y, BUTTON_W, BUTTON_H, flags);

    int B_OK_Y = LISTVIEW_Y + LISTVIEW_H/2 + (int)(BUTTON_H*2.5) + PADDING*3;
    SetWindowPos(GetDlgItem(hwnd, ID_BUTTON_OK), 0, 
        BUTTON_X, B_OK_Y, BUTTON_W, BUTTON_H, flags);

    /* Position and size the status bar*/
    SendMessage(GetDlgItem(hwnd, ID_STATUSBAR), WM_SIZE, 0, 0);

    int lparam[5];
    lparam[0] = GBL_X + GBL_W/2;
    lparam[1] = GBL_X + GBL_W;
    lparam[2] = GBR_X;
    lparam[3] = GBR_X + GBR_W/2;
    lparam[4] = -1;
    SendMessage(GetDlgItem(hwnd, ID_STATUSBAR), SB_SETPARTS,  5, (LPARAM)lparam);
}

void wm_create(HWND hwnd, HINSTANCE hInst) {
    listview_create(hwnd, hInst, ID_LISTVIEW_LEFT);
    listview_create(hwnd, hInst, ID_LISTVIEW_RIGHT);
    search_create(hwnd, hInst, ID_SEARCH_LEFT);
    search_create(hwnd, hInst, ID_SEARCH_RIGHT);
    //button_create(hwnd, hInst, TEXT(">>"), ID_BUTTON_PICK_ALL);
    button_create(hwnd, hInst, TEXT(">"), ID_BUTTON_PICK_SELECTED);
    button_create(hwnd, hInst, TEXT("<"), ID_BUTTON_UNPICK_SELECTED);
    //button_create(hwnd, hInst, TEXT("<<"), ID_BUTTON_UNPICK_ALL);
    undo_redo_create(hwnd, hInst, TEXT("Undo"), ID_BUTTON_UNDO);
    undo_redo_create(hwnd, hInst, TEXT("Redo"), ID_BUTTON_REDO);
    button_create(hwnd, hInst, TEXT("OK"), ID_BUTTON_OK);
    groupbox_create(hwnd, hInst, TEXT("Inactive Conditions"), ID_GROUPBOX_LEFT);
    groupbox_create(hwnd, hInst, TEXT("Active Conditions"), ID_GROUPBOX_RIGHT);
    status_create(hwnd, hInst, ID_STATUSBAR);
}

void wm_notify(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    struct state *s = (struct state*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    LPNMHDR lpnmhdr = (LPNMHDR)lParam;
    switch (lpnmhdr->idFrom) {
    case ID_LISTVIEW_LEFT:
    case ID_LISTVIEW_RIGHT:
        condlist *list = lpnmhdr->idFrom == ID_LISTVIEW_LEFT ? s->left : s->right;
        switch (lpnmhdr->code) {
            case NM_RCLICK:
                HMENU hmenu = LoadMenu(s->hInst, MAKEINTRESOURCE(ID_RIGHTCLICK_MENU)); 
                HMENU hmenuTrackPopup = GetSubMenu(hmenu, 0); 
                POINT pt;
                GetCursorPos(&pt);
                TrackPopupMenu(hmenuTrackPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, hwnd, NULL); 
                DestroyMenu(hmenu); 
                break;
            case LVN_GETDISPINFO:
                NMLVDISPINFO* lpdi = (NMLVDISPINFO*)lParam;
                {
                    switch(lpdi->item.iSubItem) {
                    case 0:
                        if (lpdi->item.mask & LVIF_TEXT) {
                            struct strbuf b = MEMBUF(lpdi->item.pszText, lpdi->item.cchTextMax);
                            append_long(&b, abs(condlist_get_id(list, lpdi->item.iItem)));
                            APPEND_STR(&b, L"\0");
                        }
                        break;
                    case 1:
                        MultiByteToWideChar(CP_UTF8, 0, condlist_get_title(list, lpdi->item.iItem), -1, lpdi->item.pszText, lpdi->item.cchTextMax);
                        break;
                    }
                }
                break;
            case LVN_COLUMNCLICK:
                int col_index = ((LPNMLISTVIEW)lParam)->iSubItem;
                
                /* get the header */
                HWND hHeader = ListView_GetHeader(lpnmhdr->hwndFrom);
                HDITEM hItem = { 0 };
                hItem.mask = HDI_FORMAT;
                Header_GetItem(hHeader, col_index, &hItem);
                
                switch (col_index) {
                    case 0:
                        if (hItem.fmt & HDF_SORTDOWN) { /* clear sort */
                            hItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
                            condlist_sort_clear(list);
                        } else if (hItem.fmt & HDF_SORTUP) { /* switch to sort down */
                            hItem.fmt &= ~HDF_SORTUP;
                            hItem.fmt |= HDF_SORTDOWN;
                            condlist_sort_id_descending(list);
                        } else { /* switch to sort up */
                            hItem.fmt &= ~HDF_SORTDOWN;
                            hItem.fmt |= HDF_SORTUP;
                            condlist_sort_id_ascending(list);
                            //printf("%d\n", list->isactive);
                        }
                        Header_SetItem(hHeader, col_index, &hItem);
                        hItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
                        Header_SetItem(hHeader, 1, &hItem);
                        break;
                    case 1:
                        if (hItem.fmt & HDF_SORTDOWN) { /* clear sort */
                            hItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
                            condlist_sort_clear(list);
                        } else if (hItem.fmt & HDF_SORTUP) { /* switch to sort down */
                            hItem.fmt &= ~HDF_SORTUP;
                            hItem.fmt |= HDF_SORTDOWN;
                            condlist_sort_title_descending(list);
                        } else { /* switch to sort up */
                            hItem.fmt &= ~HDF_SORTDOWN;
                            hItem.fmt |= HDF_SORTUP;
                            condlist_sort_title_ascending(list);
                        }
                        Header_SetItem(hHeader, col_index, &hItem);
                        hItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
                        Header_SetItem(hHeader, 0, &hItem);
                        break;
                }
                update(hwnd);
                break;
        }
        break;
        
    case ID_BUTTON_UNDO:
        if (lpnmhdr->code == BCN_DROPDOWN ) {
            NMBCDROPDOWN* pDropDown = (NMBCDROPDOWN*)lParam;
            POINT pt;
            pt.x = pDropDown->rcButton.left;
            pt.y = pDropDown->rcButton.bottom;
            ClientToScreen(pDropDown->hdr.hwndFrom, &pt);
    
            /* Create a menu and add items. */
            HMENU hSplitMenu = CreatePopupMenu();
            wchar_t msg[256];
            struct strbuf b = MEMBUF(msg, sizeof(msg));

            int undo_count = 0;
            for (int i=s->buf_cur-1; i >= 0; i--){
                b.len = 0;
                APPEND_STR(&b, L"Active Count: ");
                append_long(&b, s->undo_count_list[i]);
                APPEND_STR(&b, L"\0"); 
                AppendMenu(hSplitMenu, MF_BYPOSITION, ++undo_count, msg);
            }
    
            /* Display the menu. */
            undo_count = TrackPopupMenu(hSplitMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD , pt.x, pt.y, 0, hwnd, NULL);

            /* perform the undo */
            for (int i=0; i<undo_count; i++){
                state_undo(s);
            }
            update(hwnd);
        }
        break;
    case ID_BUTTON_REDO:
        if (lpnmhdr->code == BCN_DROPDOWN ) {
            NMBCDROPDOWN* pDropDown = (NMBCDROPDOWN*)lParam;
            POINT pt;
            pt.x = pDropDown->rcButton.left;
            pt.y = pDropDown->rcButton.bottom;
            ClientToScreen(pDropDown->hdr.hwndFrom, &pt);
    
            /* Create a menu and add items. */
            HMENU hSplitMenu = CreatePopupMenu();
            wchar_t msg[256];
            struct strbuf b = MEMBUF(msg, sizeof(msg));

            int redo_count = 0;
            for (int i=s->buf_cur+1; i<s->buf_used; i++){
                b.len = 0;
                APPEND_STR(&b, L"Active Count: ");
                append_long(&b, s->undo_count_list[i]);
                APPEND_STR(&b, L"\0"); 
                AppendMenu(hSplitMenu, MF_BYPOSITION, ++redo_count, msg);
            }
    
            /* Display the menu. */
            redo_count = TrackPopupMenu(hSplitMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD , pt.x, pt.y, 0, hwnd, NULL);

            /* perform the undo */
            for (int i=0; i<redo_count; i++){
                state_redo(s);
            }
            update(hwnd);
        }
        break;
    }
}

/* Handle the range selection dialog */
INT_PTR CALLBACK RangeProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG:
            SetDlgItemText(hDlg, ID_EDIT_BY, "1");
            SetDlgItemText(hDlg, ID_EDIT_FROM, "1");
            return (INT_PTR)TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                int from = GetDlgItemInt(hDlg, ID_EDIT_FROM, NULL, false);
                int to = GetDlgItemInt(hDlg, ID_EDIT_TO, NULL, false);
                int by = GetDlgItemInt(hDlg, ID_EDIT_BY, NULL, false);
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            } else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}

void wm_command(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    struct state *s = (struct state*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (LOWORD(wParam)){
        case ID_BUTTON_PICK_ALL:
            if (HIWORD(wParam) == BN_CLICKED) {
                if ( condlist_len(s->left) > 0) state_advance(s);
                for (int i = 0; i < condlist_len(s->left); i++) {
                    condlist_flip(s->left, i);
                }
                if ( condlist_len(s->left) > 0) update(hwnd);
            }
            ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_LEFT), -1, 0, LVIS_SELECTED);
            ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT), -1, 0, LVIS_SELECTED);
            break;
        case ID_BUTTON_UNPICK_ALL:
            if (HIWORD(wParam) == BN_CLICKED) {
                if ( condlist_len(s->right) > 0) state_advance(s);
                for (int i = 0; i < condlist_len(s->right); i++) {
                    condlist_flip(s->right, i);
                }
                if ( condlist_len(s->right) > 0) update(hwnd);
            }
            ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_LEFT), -1, 0, LVIS_SELECTED);
            ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT), -1, 0, LVIS_SELECTED);
            break;
        case ID_BUTTON_PICK_SELECTED:
            if (HIWORD(wParam) == BN_CLICKED) {
                int count = ListView_GetSelectedCount(GetDlgItem(hwnd, ID_LISTVIEW_LEFT));
                if (count > 0) state_advance(s);
                int index = -1;
                for (int i = 0; i < count; i++) {
                    index = ListView_GetNextItem(GetDlgItem(hwnd, ID_LISTVIEW_LEFT), index, LVNI_SELECTED);
                    condlist_flip(s->left, index);
                }
                if (count > 0) update(hwnd);
            }
            ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_LEFT), -1, 0, LVIS_SELECTED);
            ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT), -1, 0, LVIS_SELECTED);
            break;
        case ID_BUTTON_UNPICK_SELECTED:
            if (HIWORD(wParam) == BN_CLICKED) {
                int count = ListView_GetSelectedCount(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT));
                if (count > 0) state_advance(s);
                int index = -1;
                for (int i = 0; i < count; i++) {
                    index = ListView_GetNextItem(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT), index, LVNI_SELECTED);
                    condlist_flip(s->right, index);
                }
                if (count > 0) update(hwnd);
            }
            ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_LEFT), -1, 0, LVIS_SELECTED);
            ListView_SetItemState(GetDlgItem(hwnd, ID_LISTVIEW_RIGHT), -1, 0, LVIS_SELECTED);
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
        case ID_BUTTON_OK:
            s->ok = true;
            PostQuitMessage(0);
            break;
        case ID_BUTTON_UNDO:
            state_undo(s);
            update(hwnd);
            break;
        case ID_BUTTON_REDO:
            state_redo(s);
            update(hwnd);
            break;
        case ID_RANGEBOX:
            DialogBox(s->hInst, MAKEINTRESOURCEW(ID_RANGEBOX), hwnd, RangeProc);
            break;
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: 
            wm_create(hwnd, ((CREATESTRUCT*)lParam)->hInstance);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_CTLCOLORSTATIC:
            HBRUSH *hbrBkgnd = (HBRUSH*)GetClassLongPtr(hwnd, GCLP_HBRBACKGROUND);
            return (INT_PTR)hbrBkgnd;
        case WM_PAINT: 
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
            EndPaint(hwnd, &ps);
            break;
        case WM_SIZE:
            wm_size(hwnd, HIWORD(lParam), LOWORD(lParam));
            break;
        case WM_NOTIFY: 
            wm_notify(hwnd, wParam, lParam); 
            break;
        case WM_COMMAND:
            wm_command(hwnd, wParam, lParam);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return NULL;
}

int _DllMainCRTStartup(void) {
    return 1;
}

__declspec(dllexport)
int fepick_case(int *id, char **title, int n) {
    /* Get the instance for the DLL. We need to search by name, so use DLLNAME */
    HINSTANCE hinstance = GetModuleHandle(TEXT(DLLNAME));

    /* This is required to initialize the common controls (listview) */
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
    wc.hIcon         = LoadIcon(hinstance, MAKEINTRESOURCE(ID_ICON));;
    wc.hCursor       = NULL;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = TEXT(CLASSNAME);
    wc.hIconSm       = NULL;

    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        TEXT(CLASSNAME),
        TEXT("Select Active Load Cases"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 500,
        NULL, NULL, hinstance, NULL
    );

    /* Set up the struct that holds the state info for the condition list */
    struct state *s = state_create(id, title, n, hinstance);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)s);

    ShowWindow(hwnd, SW_SHOW);
    update(hwnd);

    HACCEL hAccelTable = LoadAccelerators(hinstance, MAKEINTRESOURCE(ID_ACCEL_GLOBAL));

    MSG message;
    while (GetMessage(&message, NULL, 0, 0)) {

        int retcode = 0;
        switch (GetWindowLongPtrW(hwnd,GWLP_ID)){
            case ID_SEARCH_LEFT:
            case ID_SEARCH_RIGHT:
                break;
            default:
                retcode = TranslateAccelerator(hwnd, hAccelTable, &message);
                break;
        }

        if (!retcode) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    /* Update the outputs for the function */    
    int active_count = 0;
    for (int i=0; i<n; i++) {
        if (s->ok) {
            id[i] = s->buf[s->n*s->buf_cur+i];
        }
        if (id[i] > 0) active_count++;
    }

    state_free(s);

    return active_count;
}

