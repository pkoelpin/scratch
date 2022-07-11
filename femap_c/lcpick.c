/*
 List view example code
 http://csdata.iblogger.org/programming/CodeSamples/ 

 Examples for GetWindowLongPtr
 https://docs.microsoft.com/en-us/windows/win32/learnwin32/managing-application-state-

 Checkbox info
 https://devblogs.microsoft.com/oldnewthing/20140113-00/?p=2103

 Virtual Listview
 https://docs.microsoft.com/en-us/windows/win32/controls/list-view-controls-overview?redirectedfrom=MSDN#Virtual_ListView_Style

 Using virtual lists
 https://www.codeproject.com/Articles/7891/Using-virtual-lists

 Virtual list control example
 https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/winui/controls/common/vlistvw

 Select all in list view
 https://stackoverflow.com/questions/9039989/how-to-selectall-in-a-winforms-virtual-listview
*/

#include <stdbool.h>
#include <windows.h>
#include <commctrl.h> 
#include <Uxtheme.h>
#include "lcpick.h"

#define LCPICK_CLASS "LCPICK_CLASS"

struct lcpick_t
{
    HWND hwnd;
    HWND hwnd_listview;
    size_t count;
    uint32_t *lcid;
    char **desc;
};

void lcpick_set_cond(h_lcpick lcpick, size_t count, uint32_t *lcid, char **desc) 
{
    char tmp[256];
    lcpick->count = count;
    lcpick->lcid = malloc(count * sizeof(*lcpick->lcid));
    lcpick->desc = malloc(count * sizeof(char*));

    for (int i = 0; i < count; i++){
        lcpick->lcid[i] = lcid[i];
        lcpick->desc[i] = malloc(256);
        memcpy(lcpick->desc[i], desc[i], 256);
    }

    ListView_DeleteAllItems(lcpick->hwnd_listview);
    ListView_SetItemCount(lcpick->hwnd_listview, count);
}

static void create_listview(HWND hwnd){
    HINSTANCE hInstance = GetModuleHandle(NULL);

    INITCOMMONCONTROLSEX icex;
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    RECT rcClient;
    GetClientRect (hwnd, &rcClient); 

    h_lcpick lcpick = GetWindowLongPtr(hwnd, GWLP_USERDATA);

    HWND hwnd_listview = CreateWindow(
        WC_LISTVIEW, 
        L"",
        WS_CHILD | LVS_REPORT  | WS_VISIBLE | LVS_OWNERDATA | WS_BORDER ,
        7, 7,
        rcClient.right - rcClient.left - 14,
        rcClient.bottom - rcClient.top - 14,
        hwnd,
        NULL,
        hInstance,
        NULL
    ); 

    ListView_SetExtendedListViewStyleEx(hwnd_listview, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    ListView_SetExtendedListViewStyleEx(hwnd_listview, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);

    HWND hHeader = ListView_GetHeader(hwnd_listview);
    LONG_PTR styles = GetWindowLongPtr(hHeader, GWL_STYLE);
    SetWindowLongPtr(hHeader, GWL_STYLE, styles | HDS_FILTERBAR);

    lcpick->hwnd_listview = hwnd_listview;

    /* Set up the column headers */
    LV_COLUMN lvC;
    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;
	lvC.cx = 100; 

	lvC.pszText = "Active";
	ListView_InsertColumn(hwnd_listview, 0, &lvC);
    lvC.pszText = "LCID";
 	ListView_InsertColumn(hwnd_listview, 1, &lvC);
    lvC.pszText = "Label";
	ListView_InsertColumn(hwnd_listview, 2, &lvC);
}

LRESULT lcpick_notify(HWND hwnd, LPARAM lParam)
{
    LPNMHDR lpnmh = (LPNMHDR) lParam;
    switch(lpnmh->code)
    {
        case LVN_GETDISPINFO:
        {
            LV_DISPINFO *lpdi = (LV_DISPINFO *)lParam;
            if(lpdi->item.mask & LVIF_TEXT)
            {
                sprintf(lpdi->item.pszText, "SUBCASE %d", lpdi->item.iItem);
            }
        }
        return 0;
    }

    return 0;
}

static LRESULT CALLBACK lcpick_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
            CREATESTRUCT *pCreate = (CREATESTRUCT*)lParam;
            h_lcpick lcpick = pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, lcpick);
            create_listview(hwnd);
            break;
        case WM_NOTIFY:
            return lcpick_notify(hwnd, lParam);
        case WM_SIZING:
            //SendMessage(GetWindow(hwnd, GW_OWNER), WM_NOTIFY, NULL, NULL);
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

static int lcpick_register(){
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSEX wc;
    
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = lcpick_proc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = LCPICK_CLASS;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    return RegisterClassEx(&wc);
}

h_lcpick lcpick_create(HWND hwnd)
{
    h_lcpick lcpick = malloc(sizeof(struct lcpick_t));
    HINSTANCE hInstance = GetModuleHandle(NULL);

    /* Only register the class if it has not been registered yet */
    if (!GetClassInfoEx(hInstance, LCPICK_CLASS, NULL)) {
        lcpick_register(hInstance);
    }

    /* Create the main window for the load case picker */
    lcpick->hwnd = CreateWindowEx(
        0,
        LCPICK_CLASS,
        "Active Condition Select",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 300,
        hwnd, NULL, hInstance, lcpick
    );
    return lcpick;
}

void lcpick_destroy(h_lcpick lcpick)
{
    for (int i = 0; i < lcpick->count; i++){
        free(lcpick->desc[i]);
    }
    free(lcpick->desc);
    free(lcpick->lcid);
    free(lcpick);
}