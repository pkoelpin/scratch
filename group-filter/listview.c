#include <Windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "resource.h"
#include "listview.h"
#include "entitylist.h"

HWND listview_create(HWND hwnd_parent, HINSTANCE hInstance)
{
    RECT rc;
    GetClientRect(hwnd_parent, &rc);

    HWND hwnd_listview = CreateWindowEx(
        0,
        WC_LISTVIEW,
        NULL,
        WS_CHILD | LVS_REPORT | WS_VISIBLE | LVS_OWNERDATA | WS_BORDER,
        7, 70,
        rc.right - rc.left - 14,
        rc.bottom - rc.top - 14,
        hwnd_parent, ID_LISTVIEW, hInstance, NULL
    );

    ListView_SetExtendedListViewStyleEx(hwnd_listview, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    ListView_SetExtendedListViewStyleEx(hwnd_listview, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);

    LV_COLUMN lvC;
    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvC.fmt = LVCFMT_LEFT;
    lvC.cchTextMax = 80;
    lvC.cx = 25;
    lvC.pszText = L"";
    ListView_InsertColumn(hwnd_listview, 0, &lvC);

    lvC.pszText = L"ID";
    lvC.fmt = LVCFMT_RIGHT;
    lvC.cx = 25;
    ListView_InsertColumn(hwnd_listview, 1, &lvC);

    lvC.pszText = L"Title";
    lvC.fmt = LVCFMT_LEFT;
    lvC.cx = 400;
    ListView_InsertColumn(hwnd_listview, 2, &lvC);

    /* Set the images */
    const int numImages = 3;
    const int bitmapSize = 16;
    HIMAGELIST hImageList = ImageList_Create(
        bitmapSize,
        bitmapSize,
        ILC_COLOR24 | ILC_MASK,
        numImages,
        0);

    HBITMAP hbmp;
    hbmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_LISTVIEW_CLEAR));
    ImageList_AddMasked(hImageList, hbmp, RGB(233, 236, 238));
    hbmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_LISTVIEW_SHOW));
    ImageList_AddMasked(hImageList, hbmp, RGB(233, 236, 238));
    hbmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_LISTVIEW_HIDE));
    ImageList_AddMasked(hImageList, hbmp, RGB(233, 236, 238));

    ListView_SetImageList(hwnd_listview, hImageList, LVSIL_STATE);

    return hwnd_listview;
}

void listview_notify(HINSTANCE hInst, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, entitylist* el)
{
    switch (((LPNMHDR)lParam)->code)
    {
    case LVN_GETDISPINFO:
    {
        LV_DISPINFO* lpdi = (LV_DISPINFO*)lParam;
        int id;
        int visibility;
        wchar_t* title;
        entitylist_get(el, lpdi->item.iItem, &id, &visibility, &title);
        switch (lpdi->item.iSubItem)
        {

        case 0:
            if (lpdi->item.mask & LVIF_STATE)
                lpdi->item.state = INDEXTOSTATEIMAGEMASK(visibility);
                lpdi->item.stateMask = TVIS_STATEIMAGEMASK;
            break;
        case 1:
            swprintf_s(lpdi->item.pszText, lpdi->item.cchTextMax, L"%d", id);
            break;
        case 2:
            lpdi->item.pszText = title;
            break;
        }
        break;
    }
    case NM_CLICK:
    {
        LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
        if (lpnmitem->iSubItem == 0)
        {
            entitylist_vis_advance(el, lpnmitem->iItem);
            int rc = ListView_RedrawItems(((LPNMHDR)lParam)->hwndFrom, lpnmitem->iItem, lpnmitem->iItem);
        }
        break;
    }
    return 0;
    }
    return 0;
}
