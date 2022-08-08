#include <Windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"
#include "listview.h"
#include "entitylist.h"
#include "femap.h"

static HFONT font_active;

HWND listview_create(HWND hwnd_parent, HINSTANCE hInstance)
{
    RECT rc;
    GetClientRect(hwnd_parent, &rc);

    HWND hwnd_listview = CreateWindowEx(
        0,
        WC_LISTVIEW,
        NULL,
        WS_CHILD | LVS_REPORT | WS_VISIBLE | LVS_OWNERDATA | WS_BORDER,
        0, 0, 0, 0,
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
    lvC.cx = 50;
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

    /* Set up the header */
    HWND hHeader = ListView_GetHeader(hwnd_listview);
    HDITEM hItem = { 0 };
    hItem.mask = HDI_FORMAT;
    Header_GetItem(hHeader, 1, &hItem);
    hItem.fmt |= HDF_SORTUP;
    Header_SetItem(hHeader, 1, &hItem);

    /* Set up the font for the active item*/

    font_active = SendMessage(hwnd_listview, WM_GETFONT, 0, 0);
    
    font_active = CreateFont(0, 0, 0, 0, FW_BOLD,
        0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Segoe UI");

    return hwnd_listview;
}

int listview_notify(HINSTANCE hInst, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, void* model, entitylist* el, HWND hwnd_statubar)
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
        {
            swprintf_s(lpdi->item.pszText, lpdi->item.cchTextMax, L"%d", id);
            break;
        }
        case 2:
        {
            lpdi->item.pszText = title;
            break;
        }
        }
        break;
    }
    case NM_CUSTOMDRAW:
    {
        LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
        switch (lplvcd->nmcd.dwDrawStage) {
        case CDDS_PREPAINT:
            return CDRF_NOTIFYITEMDRAW;
            break;
        case CDDS_ITEMPREPAINT:
        {
            int index = (int)lplvcd->nmcd.dwItemSpec;
            int id;
            entitylist_get(el, index, &id, NULL, NULL);

            if (id == entitylist_get_active(el)) {
                lplvcd->clrText = RGB(0, 0, 255);
                SelectObject(((HDC)lplvcd->nmcd.hdc), font_active);
                //lplvcd->clrTextBk = RGB(255, 0, 0);
            }
            return CDRF_NEWFONT;
            break;
        }
        case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
            return CDRF_NEWFONT;
            break;
        }
        return TRUE;
    }
    case NM_CLICK:
    {
        LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
        HWND hwnd_listview = ((LPNMHDR)lParam)->hwndFrom;
        if (lpnmitem->iSubItem == 0 && lpnmitem->iItem >= 0)
        {
            /* Change the visibilty state of the selected item */
            entitylist_vis_advance(el, lpnmitem->iItem);
            int visibility;
            entitylist_get(el, lpnmitem->iItem, NULL, &visibility, NULL);

            /* Check to see if the item that the user clicked is also selected */
            int index = ListView_GetNextItem(hwnd_listview, lpnmitem->iItem-1, LVNI_SELECTED);

            /* Get all selected items and upate their visibility */
            if (index == lpnmitem->iItem)
            {
                int selected_count = ListView_GetSelectedCount(hwnd_listview);
                index = -1;
                for (int i = 0; i < selected_count; i++) {
                    index = ListView_GetNextItem(hwnd_listview, index, LVNI_SELECTED);
                    entitylist_set_vis(el, index, visibility);
                }
            }

            /* Update the list view */
            ListView_RedrawItems(hwnd_listview, lpnmitem->iItem, lpnmitem->iItem);
            ListView_RedrawItems(hwnd_listview, -1, index);

            /* Set the visibility state of all the groups */
            int nGroups = entitylist_get_vis_count(el);
            int* nGroupID = malloc(nGroups * sizeof(int));
            entitylist_get_vis(el, nGroupID);
            femap_view_SetMultiGroupList(model, true, nGroups, nGroupID);
            free(nGroupID);
        }
        break;
    }
    case NM_DBLCLK:
    {
        LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
        HWND hwnd_listview = ((LPNMHDR)lParam)->hwndFrom;
        int id;
        entitylist_get(el, lpnmitem->iItem, &id, NULL, NULL);
        entitylist_set_active(el, id);
        ListView_RedrawItems(hwnd_listview, -1, entitylist_count(el));
        femap_group_SetActive(model, id);
        femap_status_redraw(model);
        //femap_view_regenerate(model);
        //InvalidateRect(hwnd_statubar, NULL, TRUE);
        break;
    }
    case LVN_COLUMNCLICK:
    {
        HWND hwnd_listview = ((LPNMHDR)lParam)->hwndFrom;
        int col_index = ((LPNMLISTVIEW)lParam)->iSubItem;

        /* Get the header */
        HWND hHeader = ListView_GetHeader(hwnd_listview);

        /* set the header */
        //HDITEM hItem = { 0 };
        //hItem.mask = HDI_FORMAT;
        //Header_GetItem(hHeader, col_index, &hItem);
        //hItem.fmt |= HDF_SORTUP;
        //Header_SetItem(hHeader, col_index, &hItem);
        break;
    }
    return 0;
    }
    return 0;
}
