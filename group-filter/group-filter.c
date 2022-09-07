// group-filter.cpp : Defines the entry point for the application.
//
// coding style conventions:
// https://docs.microsoft.com/en-us/windows/win32/stg/coding-style-conventions
//
// UX guide
// https://docs.microsoft.com/en-us/windows/win32/uxguide
// https://docs.microsoft.com/en-us/windows/win32/uxguide/vis-layout?redirectedfrom=MSDN#sizingandspacing
// 
// ID Conventions
// https://docs.microsoft.com/en-us/cpp/mfc/tn020-id-naming-and-numbering-conventions?view=msvc-170
//
// DPI Awareness
// https://docs.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows

// The following pragma enables common controls
// This compiler directive will only work in Visual C++
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdbool.h>
#include "framework.h"
#include "resource.h"
#include "group-filter.h"
#include "toolbar.h"
#include "commctrl.h"
#include "listview.h"
#include "search.h"
#include "femap.h"
#include "entitylist.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hwnd_main;
HWND hwnd_toolbar;
HWND hwnd_search;
HWND hwnd_modelinfo;
HWND hwnd_treeview;
HWND hwnd_listview;
void* femodel;
entitylist* el;
UINT WM_FEMAP_MESSAGE;

// Forward declarations of functions included in this code module:
ATOM                RegisterWindowClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK       find_modelinfo(HWND hwnd, LPARAM lParam);
BOOL CALLBACK       find_treeview(HWND hwnd, LPARAM lParam);
void                grouplist_refresh();
void                resize_all();

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // Load the common control set
    INITCOMMONCONTROLSEX icex = { 0 };
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_GROUPFILTER, szWindowClass, MAX_LOADSTRING);
    RegisterWindowClass(hInstance);

    // Initialize some global variables
    el = entitylist_create();

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GROUPFILTER));

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        int retcode = 0;
        if (msg.hwnd == hwnd_listview) {
            retcode = TranslateAccelerator(hwnd_main, hAccelTable, &msg);
        }
        if (!retcode)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {0};

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GROUPFILTER));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    hwnd_main = CreateWindowW(
        szWindowClass, 
        szTitle, 
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 500, 500, 
        NULL, NULL, hInstance, NULL);

   if (!hwnd_main)
   {
      return FALSE;
   }

   ShowWindow(hwnd_main, nCmdShow);
   UpdateWindow(hwnd_main);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_FEMAP_MESSAGE)
    {
        switch (wParam) {
        case 5:
            free(NULL);
            break;
        case 9:  // FEVENT_DRAWEND
            grouplist_refresh();
            break;
        }
        return 0;
    }
    switch (message)
    {
    case WM_CREATE:
    {
        hwnd_toolbar = toolbar_create(hWnd, hInst);
        hwnd_search = search_create(hWnd, hInst);
        hwnd_listview = listview_create(hWnd, hInst);

        /* Connect to the femap instance and register to get messages */
        femodel = femap_connect();
        HWND hwnd_femap = femap_hMainWnd(femodel);
        femap_register(femodel, hWnd);
        WM_FEMAP_MESSAGE = RegisterWindowMessage(L"FE_EVENT_MESSAGE");

        // Set the group list
        grouplist_refresh();
        
        break;
    }
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_HIGHLIGHTOPTIONS_HIGHLIGHT:
                break;
            case ID_SHOWFULLMODEL:
                break;
            case ID_SELECTALL:
                ListView_SetItemState(hwnd_listview, -1, LVIS_SELECTED, LVIS_SELECTED);
                break;
            case IDM_SEARCHBAR:
                search_command(hInst, hWnd, message, wParam, lParam, el);
                int count = entitylist_count(el);
                ListView_SetItemCount(hwnd_listview, count);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_WINDOWPOSCHANGED:
    case WM_SIZE:
        resize_all();
        break;
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->idFrom)
        {
        case ID_TOOLBAR:
            toolbar_notify(hInst, hWnd, message, wParam, lParam, femodel);
            break;
        case ID_LISTVIEW:
            return listview_notify(hInst, hWnd, message, wParam, lParam, femodel, el);
        }
        break;
    case WM_DESTROY:
        entitylist_free(el);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void resize_all()
{
    RECT rect;
    if (!GetClientRect(hwnd_main, &rect))
        return;

    int MARGIN = 11;
    int PADDING = 7;

    int TOOLBAR_X = 0;
    int TOOLBAR_Y = 0;
    int TOOLBAR_W = rect.right - 2*MARGIN;
    int TOOLBAR_H = 23;
    
    int SEARCH_X = MARGIN;
    int SEARCH_Y = TOOLBAR_Y + TOOLBAR_H + PADDING;
    int SEARCH_W = rect.right - 2 * MARGIN;
    int SEARCH_H = 25;

    int LISTVIEW_X = MARGIN;
    int LISTVIEW_Y = SEARCH_Y + SEARCH_H + PADDING;
    int LISTVIEW_W = rect.right - rect.left - 2 * MARGIN;
    int LISTVIEW_H = rect.bottom - rect.top - LISTVIEW_Y - MARGIN;
    SetWindowPos(hwnd_toolbar, 0, TOOLBAR_X, TOOLBAR_Y, TOOLBAR_W, TOOLBAR_H, SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(hwnd_search, 0, SEARCH_X, SEARCH_Y, SEARCH_W, SEARCH_H, SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(hwnd_listview, 0, LISTVIEW_X, LISTVIEW_Y, LISTVIEW_W, LISTVIEW_H, SWP_NOZORDER | SWP_NOACTIVATE);
}

void grouplist_refresh() 
{
    int count = femap_group_CountSet(femodel);
    int* id = malloc(count * sizeof(int));
    int* visibility = malloc(count * sizeof(int));
    wchar_t** title = malloc(count * sizeof(wchar_t*));

    for (int i = 0; i < count; i++)
    {
        title[i] = malloc(256 * sizeof(wchar_t));
    }
    femap_group_GetTitleList(femodel, id, title);
    femap_group_GetVisibility(femodel, id, visibility);
    entitylist_setall(el, count, id, visibility, title);
    entitylist_set_active(el, femap_group_GetActive(femodel));

    /* reset the listview */
    ListView_SetItemCount(hwnd_listview, count);

    free(id);
    free(visibility);
    for (int i = 0; i < count; i++)
    {
        free(title[i]);
    }
    free(title);
}