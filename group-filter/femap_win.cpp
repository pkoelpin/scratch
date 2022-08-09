#include "femap_win.h"
#include "femap.h"
#include <Windows.h>
#include "commctrl.h"

#define TEXT_BUFFER_SIZE 256

static HWND hwnd_modelinfo;
static HWND hwnd_treeview;
BOOL CALLBACK find_modelinfo(HWND hwnd, LPARAM lParam);
BOOL CALLBACK find_treeview(HWND hwnd, LPARAM lParam);

void femap_modelinfo_select(HWND hWnd, int count, int* id)
{
    // get the hwnd for the model info pane
	EnumChildWindows(hWnd, find_modelinfo, 0);
	EnumChildWindows(hwnd_modelinfo, find_treeview, 0);

    // Get the process that the treeview is running on 
    DWORD pid;
    GetWindowThreadProcessId(hwnd_treeview, &pid);
    HANDLE process = OpenProcess(
        PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, 
        FALSE, pid);

    // allocate space for the TVITEM on the other process
    size_t txtsize = TEXT_BUFFER_SIZE * sizeof(WCHAR);
    LPVOID tviptr = VirtualAllocEx(process, NULL, sizeof(TVITEM) + txtsize, MEM_COMMIT, PAGE_READWRITE);
    
    if (tviptr == NULL)
        return;

    // create the text pointer offset from the item
    LPWSTR ptext = (LPWSTR)((char*)tviptr + sizeof(TVITEM));

    // Get the root item of the treeview 
    int item_count = TreeView_GetCount(hwnd_treeview);
    HTREEITEM root = TreeView_GetRoot(hwnd_treeview);

    // Get the first child
    HTREEITEM child = TreeView_GetNextItem(hwnd_treeview, root, TVGN_NEXT);

    TVITEM item;
    item.hItem = root;
    item.mask = TVIF_TEXT;
    item.pszText = ptext;
    item.cchTextMax = TEXT_BUFFER_SIZE;

    // Send the message to get the item
    WriteProcessMemory(process, tviptr, &item, sizeof(TVITEM), NULL);
    SendMessage(hwnd_treeview, TVM_GETITEM, 0, (LPARAM)tviptr);
    
    // Read the text for the item
    WCHAR text[TEXT_BUFFER_SIZE];
    ReadProcessMemory(process, ptext, &text, txtsize, NULL);

    // Release the memory that we reserved
    VirtualFreeEx(process, tviptr, sizeof(TVITEM) + txtsize, MEM_RELEASE);
    CloseHandle(process);
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