#import "C:\Program Files\Siemens\Femap 2020.2 Student\femap.tlb" named_guids

#include "atlsafe.h"
#include "oleauto.h"
#include "femap.h"

void* femap_connect()
{
    HRESULT hr;
    CLSID clsid;
    IUnknown FAR* punk;

    hr = OleInitialize(NULL);
    hr = CLSIDFromString(L"femap.model", &clsid);

    if (hr == S_OK)
    {
        hr = GetActiveObject(clsid, NULL, &punk);
        if (hr == S_OK)
        {
            CComQIPtr<femap::Imodel> pModel;
            pModel = punk;
            return pModel;
        }
    }
    return nullptr;
}
 
long femap_hMainWnd(void* model)
{
    CComQIPtr<femap::Imodel> pModel = (IUnknown FAR *) model;
    return pModel->hMainWnd;
}