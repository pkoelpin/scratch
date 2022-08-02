#import "C:\Program Files\Siemens\Femap 2020.2 Student\femap.tlb" named_guids

#include "atlsafe.h"
#include "oleauto.h"
#include "femap.h"
#include "entitylist.h"
#include <set>

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

void femap_register(void* model, int hwnd) {
    CComQIPtr<femap::Imodel> pModel = (IUnknown FAR*) model;
    pModel->feAppRegisterAddInPane(true, hwnd, hwnd, false, false, 1, 2);
}

int femap_group_CountSet(void* model)
{
    CComQIPtr<femap::Imodel> pModel = (IUnknown FAR*) model;
    return pModel->feGroup->CountSet();
}

/*
the id and title arrays must be sized to the count of the groups in the model
*/
void femap_group_GetTitleList(void* model, int* id, wchar_t** title)
{
    CComQIPtr<femap::Imodel> pModel = (IUnknown FAR*) model;
    int count;
    VARIANTARG vId;
    VARIANTARG vTitle;
    VariantInit(&vId);
    VariantInit(&vTitle);
    pModel->feGroup->GetTitleList(0, 0, &count, &vId, &vTitle);

    /* extract the IDs */
    LPSAFEARRAY pSafeArray = V_ARRAY(&vId);
    LPVOID pData;
    SafeArrayAccessData(pSafeArray, &pData);
    memcpy(id, pData, count*sizeof(int));
    SafeArrayUnaccessData(pSafeArray);

    /* extract the titles */
    pSafeArray = V_ARRAY(&vTitle);
    SafeArrayAccessData(pSafeArray, &pData);
    for (int i = 0; i < count; i++) {
        BSTR s = ((BSTR*)pData)[i];
        size_t len = SysStringLen(s);
        memcpy(title[i], s, len * sizeof(wchar_t));
        title[i][len] = L'\0';
        free(NULL);
    }

    VariantClear(&vId);
    VariantClear(&vTitle);
}

void femap_view_SetMultiGroupList(void* model, bool bClear, int nGroups, const int *nGroupID)
{
    CComQIPtr<femap::Imodel> pModel = (IUnknown FAR*) model;

    /* Get the active group */
    CComQIPtr<femap::IView> pView;
    pView = pModel->feView;
    int activeView;
    pModel->feAppGetActiveView(&activeView);
    pView->Get(activeView);

    CComSafeArray<int> arr(nGroups);
    for (int i = 0; i < nGroups; i++) {
        arr.SetAt(i, nGroupID[i]);
    }
    pView->SetMultiGroupList(bClear, nGroups, CComVariant(arr));
    pView->Put(0);
    pView->Regenerate();
}

void femap_group_GetVisibility(void* model, const int* id, int* visibility)
{
    CComQIPtr<femap::Imodel> pModel = (IUnknown FAR*) model;
    femap::zReturnCode rc;

    /* Get the active group */
    CComQIPtr<femap::IView> pView;
    pView = pModel->feView;
    int activeView;
    pModel->feAppGetActiveView(&activeView);
    pView->Get(activeView);

    int count;
    CComVariant vList;
    pView->GetMultiGroupList(&count, &vList);

    std::set<int> vis_set;
    for (long i = 0; i < count; i++)
    {
        vis_set.insert(CComSafeArray<int>(vList.parray).GetAt(i));
    }

    count = femap_group_CountSet(model);
    for (int i = 0; i < count; i++)
    {
        if (vis_set.find(id[i]) != vis_set.end())
        {
            visibility[i] = VIS_SHOW;
        }
        else if (vis_set.find(-id[i]) != vis_set.end())
        {
            visibility[i] = VIS_HIDE;
        }
        else
        {
            visibility[i] = VIS_CLEAR;
        }
    }
    VariantClear(&vList);
}