/** @file model.c
*
*/
#include <combaseapi.h>
#include "model.h"

struct model_t
{
    CLSID      clsid;
    IUnknown*  p_unk_model;
    IDispatch* p_disp_model;
    IDispatch* p_disp_group;
    DISPID     dispid_group;
    DISPID     dispid_group_GetTitleList;
};

h_model model_create() 
{
    h_model model = malloc(sizeof(struct model_t));

    HRESULT hr;
    hr = OleInitialize(NULL);

    hr = CLSIDFromProgID(L"femap.model", &model->clsid);

    hr = GetActiveObject(
        &model->clsid, 
        NULL, 
        &model->p_unk_model);

    hr = model->p_unk_model->lpVtbl->QueryInterface(
        model->p_unk_model, 
        &IID_IDispatch, 
        (void**)&model->p_disp_model);
    
    LPOLESTR name =  L"feGroup";
    hr = model->p_disp_model->lpVtbl->GetIDsOfNames(
        model->p_disp_model, 
        &IID_NULL, 
        &name, 
        1, 
        LOCALE_USER_DEFAULT, 
        &model->dispid_group);

    return model;
}