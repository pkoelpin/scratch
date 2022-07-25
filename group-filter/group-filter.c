// group-filter.cpp : Defines the entry point for the application.
//
// coding style conventions:
// https://docs.microsoft.com/en-us/windows/win32/stg/coding-style-conventions
//
// UX guide
// https://docs.microsoft.com/en-us/windows/win32/uxguide
// 
// ID Conventions
// https://docs.microsoft.com/en-us/cpp/mfc/tn020-id-naming-and-numbering-conventions?view=msvc-170
//
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdbool.h>
#include "framework.h"
#include "resource.h"
#include "group-filter.h"
#include "toolbar.h"
#include "listview.h"
#include "search.h"
#include "commctrl.h"
#include "femap.h"
#include "entitylist.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hwnd_toolbar;
HWND hwnd_search;
HWND hwnd_modelinfo;
HWND hwnd_treeview;
HWND hwnd_listview;
void* femodel;
entitylist* el;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK       find_modelinfo(HWND hwnd, LPARAM lParam);
BOOL CALLBACK       find_treeview(HWND hwnd, LPARAM lParam);
void                grouplist_refresh();

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // Load the common control set
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GROUPFILTER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

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
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
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

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(
        szWindowClass, 
        szTitle, 
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 500, 500, 
        NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        hwnd_toolbar = toolbar_create(hWnd, hInst);
        hwnd_search = search_create(hWnd, hInst);
        hwnd_listview = listview_create(hWnd, hInst);

        femodel = femap_connect();
        HWND hwnd_femap = femap_hMainWnd(femodel);
        //femap_register(femodel, hWnd);

        // Set the group list
        grouplist_refresh();

        /* get the hwnd for the model info pane*/
        EnumChildWindows(hwnd_femap, find_modelinfo, 0);
        EnumChildWindows(hwnd_modelinfo, find_treeview, 0);

        /* Get the process that the treeview is running on */
        DWORD  pid;
        GetWindowThreadProcessId(hwnd_treeview, &pid);
        HANDLE process = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, false, pid);

        /* Get the root item of the treeview */
        int item_count = TreeView_GetCount(hwnd_treeview);
        HTREEITEM root = TreeView_GetSelection(hwnd_treeview);

        /* allocate space for the TVITEM on the other process*/
        size_t txtsize = 256 * 2;
        LPVOID tviptr = VirtualAllocEx(process, NULL, sizeof(TVITEM) + txtsize, MEM_COMMIT, PAGE_READWRITE);

        /* create the text pointer*/
        LPWSTR ptext = (char*)tviptr + sizeof(TVITEM);
        TVITEM item;
        item.hItem = root;
        item.mask = TVIF_TEXT;
        item.pszText = ptext;
        item.cchTextMax = 256;
        WCHAR text[256];
        WriteProcessMemory(process, tviptr, &item, sizeof(TVITEM), NULL);
        SendMessage(hwnd_treeview, TVM_GETITEM, 0, tviptr);
        ReadProcessMemory(process, ptext, &text, txtsize, NULL);

        VirtualFreeEx(process, tviptr, sizeof(TVITEM) + txtsize, MEM_RELEASE);
        CloseHandle(process);
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
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_SIZE:
    {
        /* Get the dimensions of the main window */
        RECT rect;
        GetClientRect(hWnd, &rect);

        SetWindowPos(hwnd_toolbar, 0, 0, 0, rect.right, rect.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
        SetWindowPos(hwnd_search, 0, 67, 27, rect.right - 74, 33, SWP_NOZORDER | SWP_NOACTIVATE);
        SetWindowPos(hwnd_listview, 0, 7, 70, rect.right - rect.left - 14, rect.bottom - rect.top - 77, SWP_NOZORDER | SWP_NOACTIVATE);
        break;
    }
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->idFrom)
        {
        case ID_TOOLBAR:
            toolbar_notify(hInst, hWnd, message, wParam, lParam, femodel);
            break;
        case ID_LISTVIEW:
            listview_notify(hInst, hWnd, message, wParam, lParam, el);
            break;
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

BOOL CALLBACK find_modelinfo(HWND hwnd, LPARAM lParam)
{
    WCHAR title[256];
    GetWindowTextW(hwnd, title, 256);
    if (wcscmp(title, L"Model Info") == 0) {
        hwnd_modelinfo = hwnd;
        return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK find_treeview(HWND hwnd, LPARAM lParam)
{
    if (!IsWindowVisible(hwnd))
        return TRUE;
    WCHAR classname[256];
    GetClassName(hwnd, classname, 256);
    if (wcscmp(classname, L"SysTreeView32") == 0) {
        hwnd_treeview = hwnd;
        return FALSE;
    }
    return TRUE;
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
    entitylist_set(el, count, id, visibility, title);

    /* reset the listview */
    ListView_DeleteAllItems(hwnd_listview);
    ListView_SetItemCount(hwnd_listview, count);

    free(id);
    free(visibility);
    for (int i = 0; i < count; i++)
    {
        free(title[i]);
    }
    free(title);
}