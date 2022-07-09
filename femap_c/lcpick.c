/*
 List view example code
 http://csdata.iblogger.org/programming/CodeSamples/ 

 Examples for GetWindowLongPtr
 https://docs.microsoft.com/en-us/windows/win32/learnwin32/managing-application-state-

*/
#include <stdbool.h>
#include <windows.h>
#include <commctrl.h> 
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
    lcpick->count = count;
    lcpick->lcid = malloc(count * sizeof(*lcpick->lcid));
    lcpick->desc = malloc(count * sizeof(char*));

    for (int i = 0; i < count; i++){
        lcpick->lcid[i] = lcid[i];
        lcpick->desc[i] = malloc(256);
        memcpy(lcpick->desc[i], desc[i], 256);
    }

    LVITEM lvI;

    lvI.pszText   = LPSTR_TEXTCALLBACK; 
    lvI.mask      = LVIF_TEXT | LVIF_IMAGE |LVIF_STATE;
    lvI.stateMask = 0;
    lvI.iSubItem  = 0;
    lvI.state     = 0;

    for (int i = 0; i < count; i++) {
        lvI.iItem  = i;
        ListView_InsertItem(lcpick->hwnd_listview, &lvI);
        ListView_SetItemText(
            lcpick->hwnd_listview,
            i,
            2,
            lcpick->desc[i]);
    }
}

static void create_listview(HWND hwnd){
    HINSTANCE hInstance = GetModuleHandle(NULL);
    char szText[MAX_PATH];

    INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    RECT rcClient;                       // The parent window's client area.
    GetClientRect (hwnd, &rcClient); 

    h_lcpick lcpick = GetWindowLongPtr(hwnd, GWLP_USERDATA);

    HWND hwnd_listview = CreateWindow(
        WC_LISTVIEW, 
        L"test",
        WS_CHILD | LVS_REPORT  | WS_VISIBLE,
        10, 10,
        rcClient.right - rcClient.left,
        rcClient.bottom - rcClient.top,
        hwnd,
        NULL,
        hInstance,
        NULL); 

    lcpick->hwnd_listview = hwnd_listview;

    LV_COLUMN lvC;
    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;
	lvC.cx = 75; 

	lvC.pszText = "Active";
	ListView_InsertColumn(hwnd_listview, 0, &lvC);
    lvC.pszText = "LCID";
 	ListView_InsertColumn(hwnd_listview, 1, &lvC);
    lvC.pszText = "Label";
	ListView_InsertColumn(hwnd_listview, 2, &lvC);
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
    WNDCLASSEX wc;

    HINSTANCE hInstance = GetModuleHandle(NULL);

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

HWND lcpick_hwnd(h_lcpick lcpick)
{
    return lcpick->hwnd;
}

h_lcpick lcpick_create(HWND hwnd)
{
    h_lcpick lcpick = malloc(sizeof(struct lcpick_t));
    HINSTANCE hInstance = GetModuleHandle(NULL);

    if (!GetClassInfoEx(hInstance, LCPICK_CLASS, NULL)) {
        lcpick_register(hInstance);
    }

    lcpick->hwnd = CreateWindowEx(
        0,
        LCPICK_CLASS,
        "Active Condition Select",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
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