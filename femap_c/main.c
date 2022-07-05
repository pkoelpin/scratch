#include <windows.h>
#include <combaseapi.h>
#include <unknwn.h>

const char g_szClassName[] = "myWindowClass";
HWND hwnd_listbox;
HWND hwnd_edit;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
        {
            HINSTANCE hInstance = GetModuleHandle(NULL);
            hwnd_listbox = CreateWindowExW(
                0, 
                L"LISTBOX", 
                NULL, 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_AUTOVSCROLL, 
                7, 35, 300, 200,
                hwnd, NULL, hInstance, NULL);
            hwnd_edit = CreateWindowExW(
                0, 
                L"EDIT", 
                NULL, 
                WS_CHILD | WS_VISIBLE | WS_BORDER,
                7, 7, 300, 20, 
                hwnd, NULL, hInstance, NULL);
            SetFocus(hwnd_edit);
            break;
        }
        case WM_SIZING:
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void femap_connect()
{
    HRESULT hr; 
    CLSID clsid;
    IUnknown* pUnk;
    IDispatch* pDispApp = NULL;


    hr = OleInitialize(NULL);
    hr = CLSIDFromProgID(L"femap.model", &clsid);
    hr = GetActiveObject(&clsid, NULL, &pUnk);
    hr = pUnk->lpVtbl->QueryInterface(pUnk, &IID_IDispatch, (void**)&pDispApp);

    LPOLESTR    rgszNames[1];
    rgszNames[0] = L"feGroup";
    DISPID      dispid;
    hr = pDispApp->lpVtbl->GetIDsOfNames(pDispApp, &IID_NULL, &rgszNames, 1, LOCALE_USER_DEFAULT, &dispid);

    DISPPARAMS  dispparamsNoArgs = { NULL, NULL, 0, 0 };
    VARIANT     retcode;
    hr = pDispApp->lpVtbl->Invoke(pDispApp, dispid, &IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispparamsNoArgs, &retcode, NULL, NULL);
    
    IDispatch* grpDisp = retcode.pdispVal;
    rgszNames[0] = L"GetTitleList";
    hr = grpDisp->lpVtbl->GetIDsOfNames(grpDisp, &IID_NULL, &rgszNames, 1, LOCALE_USER_DEFAULT, &dispid);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    /* First we will try to connect to a femap model */
    femap_connect();

    WNDCLASSEX wc;
    HWND hwnd;
    MSG message;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "Group Filter",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    return message.wParam;
}