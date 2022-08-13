#define SET32_IMPLEMENTATION
#define TEXT_BUFFER_SIZE 256

#include "femap_win.h"
#include "femap.h"
#include <Windows.h>
#include "commctrl.h"
#include "set32.h"

static HWND hwnd_modelinfo;
static HWND hwnd_treeview;
BOOL CALLBACK find_modelinfo(HWND hwnd, LPARAM lParam);
BOOL CALLBACK find_treeview(HWND hwnd, LPARAM lParam);

void femap_modelinfo_select(HWND hWnd, int n, int* id)
{
    // Create a set to hold the ids to select
    int z = set32_z(n);
    uint32_t* table = calloc(sizeof(*table), UINT32_C(1) << z);
    for (uint32_t i = 0; i < n; i++) {
        set32_insert(table, z, id[i]);
    }

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
    HTREEITEM node = TreeView_GetRoot(hwnd_treeview);

    // Find the "Groups" node
    WCHAR text[TEXT_BUFFER_SIZE];
    do {
        node = TreeView_GetNextItem(hwnd_treeview, node, TVGN_NEXT);

        if (node == NULL)
            break;

        TVITEM item;
        item.hItem = node;
        item.mask = TVIF_TEXT;
        item.pszText = ptext;
        item.cchTextMax = TEXT_BUFFER_SIZE;

        // Send the message to get the item
        WriteProcessMemory(process, tviptr, &item, sizeof(TVITEM), NULL);
        SendMessage(hwnd_treeview, TVM_GETITEM, 0, (LPARAM)tviptr);

        // Read the text for the item
        ReadProcessMemory(process, ptext, &text, txtsize, NULL);
    } while (wcscmp(text, L"Groups"));

    // Iterate over all the groups
    node = TreeView_GetNextItem(hwnd_treeview, node, TVGN_CHILD);
    while (node != NULL) {
        TVITEM item;
        item.hItem = node;
        item.mask = TVIF_TEXT | TVIF_STATE;
        item.pszText = ptext;
        item.cchTextMax = TEXT_BUFFER_SIZE;

        // Send the message to get the item
        WriteProcessMemory(process, tviptr, &item, sizeof(TVITEM), NULL);
        SendMessage(hwnd_treeview, TVM_GETITEM, 0, (LPARAM)tviptr);

        // Read the item and the text for the item
        ReadProcessMemory(process, tviptr, &item, sizeof(TVITEM), NULL);
        ReadProcessMemory(process, ptext, &text, txtsize, NULL);

        // extract the group id from the text. 
        // 101..Title
        int group_id = wcstol(text, NULL, 10);

        // If we found a group that's in our set
        if (set32_contains(table, z, group_id))
        {
            item.state = TVIS_SELECTED;
        }
        else
        {
            item.state = 0;
        }
        item.mask = TVIF_STATE;
        item.stateMask = TVIS_SELECTED;
        WriteProcessMemory(process, tviptr, &item, sizeof(TVITEM), NULL);
        SendMessage(hwnd_treeview, TVM_SETITEM, 0, (LPARAM)tviptr);

        // Load in the next node
        node = TreeView_GetNextItem(hwnd_treeview, node, TVGN_NEXT);
    }

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