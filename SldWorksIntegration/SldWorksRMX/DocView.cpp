// DocView.cpp : Implementation of CDocView

#include "stdafx.h"
#include "DocView.h"
#include "SwDocument.h"
#include "SwRMXMgr.h"
#include "SwUserAuthorization.h"
#include "SwRMXDialog.h"

// CDocView
void CDocView::Init(CSldWorksRMX *theApp, IModelView *iModelView, CSwDocument *iParent)
{
	userAddin = theApp;
	iModelView->AddRef();
	modelView = iModelView;
	parentDoc = iParent;
}

VARIANT_BOOL CDocView::AttachEventHandlers()
{
	// Connect to the ModelView event sink
	this->m_libid = LIBID_SldWorks;
	this->m_wMajorVerNum = userAddin->GetSldWorksTlbMajor();
	this->m_wMinorVerNum = 0;

	CModelViewEvents::_tih.m_wMajor = this->m_wMajorVerNum;

	HRESULT success = this->DispEventAdvise(modelView, &__uuidof(DModelViewEvents));
	if (success == S_OK)
		return VARIANT_TRUE;
	return VARIANT_FALSE;
}

VARIANT_BOOL CDocView::DetachEventHandlers()
{
	VARIANT_BOOL detached = VARIANT_TRUE;

	// Disconnect from the ModelView event sink
	HRESULT success = DispEventUnadvise(modelView);

	CModelViewEvents::_tih.m_plibid = &GUID_NULL;

	if (success != S_OK)
		return VARIANT_FALSE;

	CComObject<CSwDocument> *pDoc;
	pDoc = (CComObject<CSwDocument>*)parentDoc;
	detached = pDoc->DetachModelViewEventHandler(modelView);
	return detached;
}

//Event Handlers
//Called when the ModelView is destroyed
STDMETHODIMP CDocView::OnDestroy(long destroyType)
{
	// TODO: Add your implementation code here
	this->DetachEventHandlers();
	return S_OK;
}
