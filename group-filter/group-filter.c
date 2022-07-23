// group-filter.cpp : Defines the entry point for the application.
//
// coding style conventions:
// https://docs.microsoft.com/en-us/windows/win32/stg/coding-style-conventions
//
// UX guide
// https://docs.microsoft.com/en-us/windows/win32/uxguide
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdbool.h>
#include "framework.h"
#include "group-filter.h"
#include "toolbar.h"
#include "search.h"
#include "commctrl.h"
#include "femap.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hwnd_toolbar;
HWND hwnd_search;
HWND hwnd_modelinfo;
HWND hwnd_treeview;
void* femodel;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK       find_modelinfo(HWND hwnd, LPARAM lParam);
BOOL CALLBACK       find_treeview(HWND hwnd, LPARAM lParam);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GROUPFILTER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

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
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

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
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icex);

        hwnd_toolbar = toolbar_create(hWnd, hInst);
        hwnd_search = search_create(hWnd, hInst);

        femodel = femap_connect();
        HWND hwnd_femap = femap_hMainWnd(femodel);

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
        BOOL rc = WriteProcessMemory(process, tviptr, &item, sizeof(TVITEM), NULL);
        LRESULT res = SendMessage(hwnd_treeview, TVM_GETITEM, 0, tviptr);
        rc = ReadProcessMemory(process, ptext, &text, txtsize, NULL);

        VirtualFreeEx(process, tviptr, sizeof(TVITEM) + txtsize, MEM_RELEASE);
        CloseHandle(process);

    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
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
        RECT cr;
        GetClientRect(hWnd, &cr);

        /* resize the toolbar */
        SetWindowPos(hwnd_toolbar, 0, 0, 0, cr.right, cr.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
        SetWindowPos(hwnd_search, 0, 67, 27, cr.right - 74, 33, SWP_NOZORDER | SWP_NOACTIVATE);

        break;

    }
    case WM_NOTIFY:
        toolbar_notify(hInst, hWnd, message, wParam, lParam, femodel);
        break;
    case WM_DESTROY:
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

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
