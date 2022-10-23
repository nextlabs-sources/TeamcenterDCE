// SwDocument.cpp : Implementation of CSwDocument
// SAN - 02282012 - Removed redundant if statements in AttachModelViewEventHandlers

#include "stdafx.h"
#include "SwDocument.h"
#include "SldWorksRMX.h"
#include "DocView.h"
#include <set>
#include <iterator>
#include "FileUtils.h"
#include "SwRMXMgr.h"
#include "SwFileTypes.h"
#include "SwUserAuthorization.h"
#include "SwUtils.h"
#include "SwResult.h"
#include "SwRMXDialog.h"
#include "SwViewOverlay.h"

class CModelDocReloader {
public:
	CModelDocReloader(const wchar_t* fileName) :_fileName(fileName) {
		_modelDoc = theSwUtils->GetModelDocByFileName(_fileName);
		if (_modelDoc)
		{
			_modelDoc->GetType(&_type);
		}
	}
	~CModelDocReloader() {
		Reload();
	}
	bool Reload() {
		if (_modelDoc != nullptr && _released) {
			long error;
			CComBSTR combstr((int)_fileName.size(), _fileName.c_str());
			VARIANT_BOOL bDirty = VARIANT_FALSE;
			_modelDoc->GetSaveFlag(&bDirty);
			LOG_DEBUG_FMT("GetSaveFlag('%s')=%d", _fileName.c_str(), bDirty);
			LOG_DEBUG_FMT("Reloading('%s')...", _fileName.c_str());
			if (SUCCEEDED(_modelDoc->ReloadOrReplace(VARIANT_FALSE, combstr, VARIANT_FALSE, &error)))
			{
				_released = false;
				auto refers = theSwUtils->GetReferencedFilesOfModelDoc(_modelDoc);
				for (auto refdoc : refers) {
					LOG_DEBUG_FMT("Reference:'%s'", refdoc.c_str());
				}
				LOG_DEBUG_FMT("ReloadOrReplace('%s') Succeeded(%ld)!", _fileName.c_str(), error);
				return true;
			}
			else {
				LOG_DEBUG_FMT("ReloadOrReplace('%s') FAILED(%ld)!", _fileName.c_str(), error);
			}
		}
		return false;
	}
	bool ReleaseLocks() {
		if (_modelDoc == nullptr)
			return false;
		VARIANT_BOOL bDirty = VARIANT_FALSE;
		_modelDoc->GetSaveFlag(&bDirty);
		LOG_DEBUG_FMT("GetSaveFlag('%s')=%d", _fileName.c_str(), bDirty);
		auto refers = theSwUtils->GetReferencedFilesOfModelDoc(_modelDoc);
		for (auto refdoc : refers) {
			LOG_DEBUG_FMT("Reference:'%s'", refdoc.c_str());
			auto refModelDoc = theSwUtils->GetModelDocByFileName(refdoc);
			if (refModelDoc) {
				refModelDoc->GetSaveFlag(&bDirty);
				LOG_DEBUG_FMT("==>GetSaveFlag=%d", bDirty);
			}
		}
		switch (_type) {
			case swDocDRAWING:
				return ReleaseDrawing();
				break;
			case swDocPART:
			case swDocASSEMBLY:
				return ReleasePartAssembly();
				break;
			default:
				return false;
				break;
		}
	}
private:
	bool ReleaseDrawing() {
		if (_type == swDocDRAWING)
		{
			long status;
			IModelDoc2* pnewdoc;
			LOG_DEBUG_FMT("ClosingDrawing('%s')...", _fileName.c_str());
			//TODO:check if file is dirty?
			if (SUCCEEDED(CSwUtils::GetUserAddin()->GetSldWorksPtr()->CloseAndReopen(_modelDoc,
				swCloseReopenOption_MatchSheet | swCloseReopenOption_DiscardChanges, &pnewdoc, &status)))
			{
				_modelDoc = pnewdoc;

				LOG_DEBUG_FMT("DrawingReopened('%s')", _fileName.c_str());
				return true;
			}
			else {
				LOG_DEBUG_FMT("CloseAndReopen('%s') failed(%;d)", _fileName.c_str(), status);
			}
		}
		return false;
	}
	bool ReleasePartAssembly() {
		if (_type == swDocPART
			|| _type == swDocASSEMBLY)
		{
			long status;
			LOG_DEBUG_FMT("ForceReleaseLocks('%s')...", _fileName.c_str());
			if (SUCCEEDED(_modelDoc->ForceReleaseLocks(&status)))
			{
				_released = true;
				LOG_DEBUG_FMT("ForceReleaseLocks('%s') Succeeded(%ld)!", _fileName.c_str(), status);
				return true;
			}
			else {
				LOG_DEBUG_FMT("ForceReleaseLocks('%s') FAILED(%ld)!", _fileName.c_str(), status);
			}
		}
		return false;
	}
	long _type = swDocNONE;
	bool _released = false;
	CComPtr<IModelDoc2> _modelDoc;
	wstring _fileName = L"";
};

CSwDocument::CSwDocument(): m_fileProtected(false), m_fileName(wstring(L""))
{
	return;
}

CSwDocument::~CSwDocument()
{
	assert(openModelViews.size() == 0);

	return;
}

void CSwDocument::Init(CSldWorksRMX* addinPtr, IModelDoc2* modDoc)
{
	LOG_INFO("CSwDocument::Init");
	userAddin = addinPtr;
	iSwApp = userAddin->GetSldWorksPtr();
	iDocument = modDoc;
	iDocument->GetType(&type);

	HRESULT hr = S_OK;
	CComBSTR  szPathName;
	hr = modDoc->GetPathName(&szPathName);
	m_fileName = szPathName;
	//if szPathName is an empty string , it means it is a new file.
	LOG_DEBUG_FMT("pModelDoc=%p FileName='%s'", iDocument.p, m_fileName.c_str());
	//Get the name of the file
	m_isvirtual = theSwUtils->TryFindRootFileOfVirtualModel(iDocument, m_physicalFile);
	if (m_fileName.size() != 0) {
		bool ret;
		CSwRMXMgr* swRMXMgrInstance = CSwRMXMgr::GetInstance();
		ret = swRMXMgrInstance->RMX_RPMGetFileStatus(m_physicalFile,&m_fileProtected);
		if (m_fileProtected) {
			//Retrieve the tags assigned to the file
			ret = swRMXMgrInstance->GetTags(m_physicalFile, m_fileTags);
			if (!ret) {
				LOG_ERROR_FMT("Error reading tags for the file-'%s'", m_fileName.c_str());
			}
			LOG_INFO_FMT("The file to be opened is protected = '%s' , File Tags = '%s'" ,m_fileName.c_str(),m_fileTags.c_str());

			/*
			//Check if file doesn't has RMX_RIGHT_EDIT, if no then set the file as Read-only.
			if (!CSwUserAuthorization::GetInstance()->IsFileReadOnly(m_fileName)) {
				if (!CSwUtils::GetInstance()->SetFileAsReadOnly(m_fileName)) {
					LOG_INFO("CSwDocument::Init. Couldn't set the file as ReadOnly", m_fileName);
				}
				LOG_INFO("CSwDocument::Init. Successfully set the file as ReadOnly", m_fileName);
			}
			*/
			/*TODO:TBD:sometimes the file may lock and cannot be protected
			if (isvirtual) {
				//protect the tmp file
				LOG_DEBUG_FMT("Protecting the tmp file of virtual model:'%s'", m_fileName.c_str());
				swRMXMgrInstance->ProtectFile(m_fileName, m_fileTags, true);
			}
			*/
		}
		else {
			LOG_DEBUG("The file to be opened is not protected :" << m_fileName.c_str());
		}
	}
}

//Returns the document type
long CSwDocument::GetType()
{
	LOG_DEBUG("CSwDocument::GetType()");
	return type;
}

//Listen for the events fired by this document
VARIANT_BOOL CSwDocument::AttachEventHandlers()
{
	VARIANT_BOOL attached = VARIANT_TRUE;

	//Connect to Document Events
	HRESULT success = E_FAIL;
	switch (type)
	{
	case swDocPART:
	{
		//Listen for PartDoc events
		CPartEvents::m_wMajorVerNum = userAddin->GetSldWorksTlbMajor();
		CPartEvents::_tih.m_wMajor = CPartEvents::m_wMajorVerNum;

		success = CPartEvents::DispEventAdvise(iDocument, &__uuidof(DPartDocEvents));
		break;
	}
	case swDocASSEMBLY:
	{
		//Listen for AssemblyDoc events
		CAssemblyEvents::m_wMajorVerNum = userAddin->GetSldWorksTlbMajor();
		CAssemblyEvents::_tih.m_wMajor = CAssemblyEvents::m_wMajorVerNum;

		success = CAssemblyEvents::DispEventAdvise(iDocument, &__uuidof(DAssemblyDocEvents));
		break;
	}
	case swDocDRAWING:
	{
		//Listen for DrawingDoc events
		CDrawingEvents::m_wMajorVerNum = userAddin->GetSldWorksTlbMajor();
		CDrawingEvents::_tih.m_wMajor = CDrawingEvents::m_wMajorVerNum;

		success = CDrawingEvents::DispEventAdvise(iDocument, &__uuidof(DDrawingDocEvents));
		break;
	}
	default:
		return VARIANT_FALSE;
	}
	if (success != S_OK)
		return VARIANT_FALSE;


	attached = this->AttachModelViewEventHandlers();
	return attached;
}

VARIANT_BOOL CSwDocument::AttachModelViewEventHandlers()
{
	VARIANT_BOOL attached = VARIANT_TRUE;
	VARIANT_BOOL         bIsVisible = VARIANT_FALSE;
	CComPtr<IModelDoc2>  swActiveDocument;

	iDocument->get_Visible(&bIsVisible);

	iSwApp->get_IActiveDoc2(&swActiveDocument);

	if (swActiveDocument == NULL)
	{
		return VARIANT_FALSE;
	}

	CComQIPtr<IUnknown, &IID_IUnknown>  pUnk1(swActiveDocument);
	CComQIPtr<IUnknown, &IID_IUnknown>  pUnk2(iDocument);

	// Connect event handlers to all previously open model views on this document
	CComPtr<IModelView> iModelView;
	iDocument->IGetFirstModelView(&iModelView);
	while (iModelView != NULL)
	{
		attached = AttachEventsToModelView(iModelView);
		if (!attached)
			return VARIANT_FALSE;
		
		CComPtr<IModelView> iNextModelView;
		iModelView->IGetNext(&iNextModelView);
		iModelView = iNextModelView;
	}

	return attached;
}

VARIANT_BOOL CSwDocument::AttachEventsToModelView(CComPtr<IModelView> iModelView) {
	VARIANT_BOOL attached = VARIANT_TRUE;
	TMapIUnknownToModelView::iterator iter;
	iter = openModelViews.find(iModelView);
	if (iter == openModelViews.end())
	{
		//Create the ModelView handler
		CComObject<CDocView> *pMView;
		CComObject<CDocView>::CreateInstance(&pMView);
		pMView->Init(userAddin, iModelView, this);

		//Attach its event handlers
		attached = pMView->AttachEventHandlers();
		if (!attached)
			return VARIANT_FALSE;

		//Add it to the list of open ModelViews
		openModelViews.insert(openModelViews.end(), TMapIUnknownToModelView::value_type(iModelView, pMView));
		pMView->AddRef();
	}

	return attached;
}

//Stop listening for the events fired by this document
VARIANT_BOOL CSwDocument::DetachEventHandlers()
{
	//Disconnect from the Document Events
	HRESULT success = E_FAIL;
	switch (type)
	{
	case swDocPART:
	{
		//Stop listening for PartDoc events
		success = CPartEvents::DispEventUnadvise(iDocument, &__uuidof(DPartDocEvents));

		CPartEvents::_tih.m_plibid = &GUID_NULL;

		break;
	}
	case swDocASSEMBLY:
	{
		//Stop listening for AssemblyDoc events
		success = CAssemblyEvents::DispEventUnadvise(iDocument, &__uuidof(DAssemblyDocEvents));

		CAssemblyEvents::_tih.m_plibid = &GUID_NULL;

		break;
	}
	case swDocDRAWING:
	{
		//Stop listening for DrawingDoc events
		success = CDrawingEvents::DispEventUnadvise(iDocument, &__uuidof(DDrawingDocEvents));

		CDrawingEvents::_tih.m_plibid = &GUID_NULL;

		break;
	}
	default:
		return VARIANT_FALSE;
	}

	//Disconnect all model view event handlers
	TMapIUnknownToModelView::iterator iter;

	for (iter = openModelViews.begin(); iter != openModelViews.end(); /*iter++*/)
	{
		TMapIUnknownToModelView::value_type obj = *iter;
		CComObject<CDocView> *pMView = (CComObject<CDocView>*)obj.second;
		iter++;
		pMView->DetachEventHandlers();
	}

	userAddin->DetachModelEventHandler(iDocument);
	return VARIANT_TRUE;
}

//Stop listening for the events fired by the specified ModelView event handler
VARIANT_BOOL CSwDocument::DetachModelViewEventHandler(IUnknown *iUnk)
{
	TMapIUnknownToModelView::iterator iter;

	iter = openModelViews.find(iUnk);
	if (iter != openModelViews.end())
	{
		TMapIUnknownToModelView::value_type obj = *iter;
		obj.first->Release();
		CComObject<CDocView> *pMView = (CComObject<CDocView>*)obj.second;
		pMView->Release();
		openModelViews.erase(iter);
	}
	return VARIANT_TRUE;
}

//attach to a component if it becomes resolved
HRESULT CSwDocument::ComponentStateChange(LPDISPATCH componentModel, long newCompState)
{
	if (componentModel == NULL)
		return S_OK;

	CComPtr<IModelDoc2> modDoc;

	componentModel->QueryInterface(__uuidof(IModelDoc2), reinterpret_cast<void**>(&modDoc));

	swComponentSuppressionState_e newState = (swComponentSuppressionState_e)newCompState;

	switch (newState)
	{
	case swComponentSuppressionState_e::swComponentFullyResolved:
	{
		VARIANT_BOOL attached = VARIANT_TRUE;
		USES_CONVERSION;

		if (modDoc != NULL)
		{
			TMapIUnknownToDocument::iterator iter;
			TMapIUnknownToDocument openDocs = userAddin->OpenDocumentsTable();
			iter = openDocs.find(modDoc);
			if (iter == openDocs.end())
			{
				attached = userAddin->AttachModelEventHandler(modDoc);
			}
		}
		modDoc.Release();
		modDoc = NULL;
	}
	break;

	case swComponentSuppressionState_e::swComponentResolved:
	{
		VARIANT_BOOL attached = VARIANT_TRUE;
		USES_CONVERSION;

		if (modDoc != NULL)
		{
			TMapIUnknownToDocument::iterator iter;
			TMapIUnknownToDocument openDocs = userAddin->OpenDocumentsTable();
			iter = openDocs.find(modDoc);
			if (iter == openDocs.end())
			{
				attached = userAddin->AttachModelEventHandler(modDoc);
			}
		}
		modDoc.Release();
		modDoc = NULL;
	}
	break;

	default:
		break;
	}
	return S_OK;
}


#pragma region PartDocEventHandlers

STDMETHODIMP CSwDocument::OnPartDestroy(int destroyType)
{
	//Clear overlay only when this is the last visible document in current SolidWorks Session.
	if (CSwUtils::GetInstance()->IsLastVisibleDocument(m_fileName)) {
		LOG_INFO("Clearing overlay for the last visible document. FileName = " << m_fileName.c_str());
		CSwViewOverlay swOverlayObj;
		swOverlayObj.ClearViewOverlay();
	}
	this->DetachEventHandlers();
	return S_OK;
}

STDMETHODIMP CSwDocument::OnPartFileSave(BSTR fileName) {
	LOG_DEBUG("FileName = " << fileName);
	CSwAuthResult authResultObj(fileName);
	if (!CSwUserAuthorization::GetInstance()->OnFileSaveAccessCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		return S_FALSE;
	}
	if (theSwUtils->IsVirtualModel(iDocument)
		&& !CanSaveVirtual(fileName, true))
	{
		return S_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CSwDocument::OnPartFileSaveAs(BSTR fileName) {
	LOG_DEBUG("FileName = " << fileName);
	CSwAuthResult authResultObj(fileName);
	if (!CSwUserAuthorization::GetInstance()->OnFileSaveAsAccessCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		return S_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CSwDocument::OnPartFilePostSave(int saveType, BSTR  fileName) {
	LOG_DEBUG_FMT("[%p]SaveType=%d , FileName='%s'", iDocument.p, saveType, fileName);
	if (!PostFileSaveHandler(saveType, fileName)) {
		return S_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CSwDocument::OnPartFileModify(void) {
	LOG_DEBUG("CSwDocument::OnPartFileModify called");
	CSwAuthResult authResultObj(m_fileName);
	if (!CSwUserAuthorization::GetInstance()->OnFileModifyAccessCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		return S_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CSwDocument::OnPartFileSavePostCancelNotify(void) {
	LOG_DEBUG("CSwDocument::OnPartFileSavePostCancelNotify called");
	//To handle the use case File->Print3D->Save To File->*.amf -> Press Save File.. . Because in this case the swAppCommandCloseNotify will not be fired
	CSldWorksRMX::ProcessRecords();
	return S_OK;
}

STDMETHODIMP CSwDocument::OnPartViewNewNotify(LPDISPATCH swObject) {
	//Use Cases : 1)Open a part file, Go to Window->New Window , 2)Open a assembly document, Right click on part in assembly and select Open part.
	if (!OnModelDocViewNotify(swObject))
		return S_FALSE;
	return S_OK;
}

#pragma endregion PartDocEventHandlers

#pragma region AssemblyDocEventHandlers

STDMETHODIMP CSwDocument::OnAssemblyDestroy(int destroyType)
{
	//Clear overlay only when this is the last visible document in current SolidWorks Session.
	if (CSwUtils::GetInstance()->IsLastVisibleDocument(m_fileName)) {
		LOG_INFO("Clearing overlay for the last visible document. FileName = " << m_fileName.c_str());
		CSwViewOverlay swOverlayObj;
		swOverlayObj.ClearViewOverlay();
	}
	this->DetachEventHandlers();
	return S_OK;
}

STDMETHODIMP CSwDocument::OnAssemblyComponentStateChangeNotify2(LPDISPATCH componentModel, BSTR compName, long oldCompState, long newCompState)
{
	return ComponentStateChange(componentModel, newCompState);
}

STDMETHODIMP CSwDocument::OnAssemblyComponentStateChangeNotify(LPDISPATCH componentModel, long oldCompState, long newCompState)
{
	return ComponentStateChange(componentModel, newCompState);
}

STDMETHODIMP CSwDocument::OnAssemblyComponentVisualPropertiesChangeNotify(LPDISPATCH swObject)
{
	if (swObject == NULL)
		return S_OK;

	CComPtr<IModelDoc2> modDoc;
	CComPtr<IComponent2> component;

	swObject->QueryInterface(__uuidof(IComponent2), reinterpret_cast<void**>(&component));

	component->IGetModelDoc(&modDoc);
	swComponentSuppressionState_e newState = swComponentSuppressionState_e::swComponentFullyResolved;

	return ComponentStateChange(modDoc, newState);
}

STDMETHODIMP CSwDocument::OnAssemblyComponentDisplayStateChangeNotify(LPDISPATCH swObject)
{
	if (swObject == NULL)
		return S_OK;

	CComPtr<IModelDoc2> modDoc;
	CComPtr<IComponent2> component;

	swObject->QueryInterface(__uuidof(IComponent2), reinterpret_cast<void**>(&component));

	component->IGetModelDoc(&modDoc);
	swComponentSuppressionState_e newState = swComponentSuppressionState_e::swComponentFullyResolved;

	return ComponentStateChange(modDoc, newState);
}
bool CSwDocument::CanSaveVirtual(const wstring&fileName, bool silent) const {
	//current document must be virtual when call this method;
	LOG_DEBUG_FMT("[%p]'%s':CachedIsVirtual=%d CachedIsProtected=%d", iDocument, m_fileName.c_str(), CachedIsVirtual(), CachedIsProtected());
	if (!CachedIsVirtual()
		&& CachedIsProtected()) {
		//
		LOG_DEBUG_FMT("Checking exract right as '%s' is made virtual from protected file", fileName.c_str());
		LOG_DEBUG("Currently always deny make virtual");
		if (silent) {
			LOG_DEBUG("Deny silently");
		}
		else
		{
			CSwAuthResult authResult(fileName);
			authResult.SetAuthResult(false);
			authResult.SetErrorMsg(L"Operation Denied.\r\n Following files are made virtual from NextLabs protected file, currently this is not supported .\r\n");
			authResult.AddFilesToDenyAccessList(FileUtils::GetFileNameWithExt(authResult.FileName()));
			CSldWorksRMXDialog rmxDialogObj;
			rmxDialogObj.ShowMessageDialog(&authResult);
		}
		return false;
	}
	return true;
}

STDMETHODIMP CSwDocument::OnAssemblyFileSave(BSTR fileName) {
	LOG_DEBUG_FMT("[%p]FileName='%s'", iDocument.p, fileName);
	CSwAuthResult authResultObj(fileName);
	if (!CSwUserAuthorization::GetInstance()->OnFileSaveAccessCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		return S_FALSE;
	}
	if (!theSwUtils->IsVirtualModel(iDocument))
	{
		LOG_TRACE_FMT("'%s' is NOT virtual", m_fileName.c_str());
		auto dependentFiles = theSwUtils->GetReferencedFilesOfModelDoc(iDocument);
		//in managed mode, the PartSave event is always called event AssemblySave is denied
		//therefore the deny message in AssemblySave is duplicated, lets keep it silent
		for (auto dfile : dependentFiles) {
			auto ddoc = theSwUtils->GetModelDocByFileName(dfile);
			wstring physicalFile;
			if (theSwUtils->TryFindRootFileOfVirtualModel(ddoc, physicalFile)
				&& physicalFile == fileName) {
				auto docInfo = userAddin->FindDocumentInfo(ddoc);
				if (docInfo != nullptr
					&& !docInfo->CanSaveVirtual(dfile, m_denyNotified)) {
					m_denyNotified = true;
					return S_FALSE;
				}
			}
		}
	}
	//always show deny message
	else if(!CanSaveVirtual(fileName, true)) {
		return S_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CSwDocument::OnAssemblyFileSaveAs(BSTR fileName) {
	LOG_DEBUG("FileName = " << fileName);

	if(m_fileName.empty() && !CSwUserAuthorization::GetInstance()->OnNewFileSaveAsAccessCheck(fileName)){
	    //Save the new file to default save location.
		wstring fileLocation = userAddin->GetDefaultSaveFolder();
		//Get the  available fileName in format Assem*.SLDASM
		wstring fileExt = FileUtils::GetFileExt(fileName);
		wstring newFileName = FileUtils::GetAvailableFileName(fileLocation,L"Assem",fileExt);
		CComBSTR bstrNewFileName((int)newFileName.size(), newFileName.data());
		iDocument->SetSaveAsFileName(bstrNewFileName);
		m_fileName = newFileName;
		return S_FALSE;
	}
	CSwAuthResult authResultObj(fileName);
	if (!CSwUserAuthorization::GetInstance()->OnFileSaveAsAccessCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		return S_FALSE;
	}
	
	return S_OK;
}

STDMETHODIMP CSwDocument::OnAssemblyFilePostSave(int saveType, BSTR  fileName) {
	LOG_DEBUG_FMT("[%p]SaveType=%d , FileName='%s'", iDocument.p, saveType, fileName);
	m_denyNotified = false;
	if (!PostFileSaveHandler(saveType, fileName)) {
	    return S_FALSE;
	}
	
	return S_OK;
}

STDMETHODIMP CSwDocument::OnAssemblyFileModify(void)
{
	LOG_DEBUG("CSwDocument::OnAssemblyModify called");
	CSwAuthResult authResultObj(m_fileName);
	if (!CSwUserAuthorization::GetInstance()->OnFileModifyAccessCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		return S_FALSE;
	}
	return S_OK;

}

STDMETHODIMP CSwDocument::OnAssemblyFileSavePostCancelNotify(void) {
	LOG_DEBUG("CSwDocument::OnAssemblyFileSavePostCancelNotify called");
	m_denyNotified = false;
	CSldWorksRMX::ProcessRecords();
	return S_OK;
}

STDMETHODIMP CSwDocument::OnAssemblyAddItemNotify(int entityType, BSTR itemName) {
	LOG_DEBUG_FMT("EntityType = %d, ItemNme = %s", entityType, itemName);
	if (entityType == swNotifyComponent) {
		AttachModelEventsToAllReferences();
	}
	return S_OK;

}

STDMETHODIMP CSwDocument::OnAssemblyViewNewNotify(LPDISPATCH swObject) {
	//Use Cases : 1)Open a assembly file, Go to Window->New Window.
	if (!OnModelDocViewNotify(swObject))
		return S_FALSE;
	return S_OK;
}

#pragma endregion AssemblyDocEventHandlers

#pragma region DrawingDocEventHandlers

STDMETHODIMP CSwDocument::OnDrawingDestroy(int destroyType)
{
	//Clear overlay only when this is the last visible document in current SolidWorks Session.
	if (CSwUtils::GetInstance()->IsLastVisibleDocument(m_fileName)) {
		LOG_INFO("Clearing overlay for the last visible document. FileName = " << m_fileName.c_str());
		CSwViewOverlay swOverlayObj;
		swOverlayObj.ClearViewOverlay();
	}
	this->DetachEventHandlers();
	return S_OK;
}

STDMETHODIMP CSwDocument::OnDrawingFileSave(BSTR fileName) {
	LOG_DEBUG("FileName = " << fileName);
	CSwAuthResult authResultObj(fileName);
	if (!CSwUserAuthorization::GetInstance()->OnFileSaveAccessCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		return S_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CSwDocument::OnDrawingFileSaveAs(BSTR fileName) {
	LOG_DEBUG("FileName = " << fileName);
	if (m_fileName.empty() && !CSwUserAuthorization::GetInstance()->OnNewFileSaveAsAccessCheck(fileName)) {
		//Save the new file to default save location.
		wstring fileLocation = userAddin->GetDefaultSaveFolder();
		//Get the  available fileName in format Draw*.SLDDRW
		wstring fileExt = FileUtils::GetFileExt(fileName);
		wstring newFileName = FileUtils::GetAvailableFileName(fileLocation, L"Draw", fileExt);
		CComBSTR bstrNewFileName((int)newFileName.size(), newFileName.data());
		iDocument->SetSaveAsFileName(bstrNewFileName);
		m_fileName = newFileName;
		return S_FALSE;
	}
	CSwAuthResult authResultObj(fileName);
	if (!CSwUserAuthorization::GetInstance()->OnFileSaveAsAccessCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		return S_FALSE;
	}

	return S_OK;
}

STDMETHODIMP CSwDocument::OnDrawingFilePostSave(int saveType, BSTR  fileName) {
	LOG_DEBUG_FMT("[%p]SaveType=%d , FileName='%s'", iDocument.p, saveType, fileName);
	if (!PostFileSaveHandler(saveType, fileName)) {
		return S_FALSE;
	}
	return S_OK;

}

STDMETHODIMP CSwDocument::OnDrawingFileModify(void)
{
	LOG_DEBUG("CSwDocument::OnDrawingFileModify called");
	CSwAuthResult authResultObj(m_fileName);
	if (!CSwUserAuthorization::GetInstance()->OnFileModifyAccessCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		return S_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CSwDocument::OnDrawingAddItemNotify(int entityType, BSTR itemName) {
	LOG_DEBUG_FMT("EntityType = %d, ItemNme = %s", entityType, itemName);
	if (entityType == swNotifyDrawingView) {
		AttachModelEventsToAllReferences();
		CSwViewOverlay::RefreshActiveWindow();
	}
	return S_OK;

}

STDMETHODIMP CSwDocument::OnDrawingViewNewNotify(LPDISPATCH swObject) {
	//Use Cases : 1)Open a drawing file, Go to Window->New Window.
	if (!OnModelDocViewNotify(swObject))
		return S_FALSE;
	return S_OK;
}

#pragma endregion DrawingDocEventHandlers


bool CSwDocument::PostFileSaveHandler(int saveType, BSTR  fileName) {
	SwFileHandlerMethod handlerType;
	bool ret = true;
	bool status = false;
	bool newFileLoaded = true;
	wstring physicalFile;
	LOG_DEBUG_FMT("[%p]CurrentName='%s' isVirtual=%d isProtected=%d ", iDocument.p, m_fileName.c_str(), m_isvirtual, m_fileProtected);
	bool isvirtual = theSwUtils->TryFindRootFileOfVirtualModel(iDocument, physicalFile);
	if (isvirtual != m_isvirtual) {
		LOG_DEBUG_FMT("[%p]IsVirtual changed from %d to %d", iDocument.p, m_isvirtual, isvirtual);
	}
	if (m_physicalFile != physicalFile) {
		LOG_DEBUG_FMT("[%p]PhysicalFile changed from '%s' to '%s'", iDocument.p, m_physicalFile.c_str(), physicalFile.c_str());
	}
	if (m_fileName != fileName)
	{
		auto newdoc = theSwUtils->GetModelDocByFileName(fileName);
		LOG_DEBUG_FMT("NewModelDoc=%p", newdoc.p);
		if (newdoc) {
			theSwUtils->TryFindRootFileOfVirtualModel(newdoc);
		}
		else {
			//as current observation, this will happened when SaveType=4
			newFileLoaded = false;
		}
	}
	switch (saveType) {
	case swFileSave:
	case swFileSaveAs:
		if (SwFileTypes::isNativeFileFormat(FileUtils::GetFileExt(fileName))) {
			m_fileName = fileName;
			if (!isvirtual)
			{
				if (m_fileProtected)
				{
					bool isprotected;
					if (CSwRMXMgr::GetInstance()->RMX_RPMGetFileStatus(fileName, &isprotected)
						&& !isprotected) {
						LOG_DEBUG_FMT("FileNotProtected('%s'):CopyFile hook may not work well", fileName);
						//TODO:for Save, when saving an assembly contains virtual models, 
						//somehow the changes will be lost after reload
						if (saveType == swFileSaveAs)
						{
							LOG_DEBUG("save as or save external happening here...");
							CModelDocReloader reloader(fileName);
							reloader.ReleaseLocks();
							if (!CSwRMXMgr::GetInstance()->ProtectFile(fileName, m_fileTags)) {
								LOG_DEBUG_FMT("Failed to protect('%s') with tags('%s')", fileName, m_fileTags.c_str());
								ret = false;
							}
							else
							{
								LOG_DEBUG_FMT("Protected('%s') with tags('%s')", fileName, m_fileTags.c_str());
							}
						}
						else {
							//NOTE:when saving assembly containing virtual part, the file lost protection but the file name still have .nxl
							//if we protect the file base on this file, somehow the new protected file contains the old version of the file
							//we need use CopyFile to create a copy of the file, and protect it
							wstring bakfile = wstring(fileName) + L".baktmp";
							LOG_DEBUG_FMT("CopyFile('%s', '%s')...", fileName, bakfile.c_str());
							if (FileUtils::CopyFileUtil(fileName, bakfile.c_str()))
							{
								wstring nxlFile;
								if (CSwRMXMgr::GetInstance()->ProtectFileAsNXL(bakfile, nxlFile, m_fileTags)
									&& FileUtils::IsFile(nxlFile)) {
									LOG_DEBUG_FMT("Protected('%s'):'%s'", bakfile.c_str(), nxlFile.c_str());
									CModelDocReloader reloader(fileName);
									reloader.ReleaseLocks();
									CSwRMXMgr::GetInstance()->RMX_DeleteFile(fileName);

									if (!MoveFile(nxlFile.c_str(), fileName)) {
										auto error = GetLastError();
										LOG_ERROR_FMT("MoveFile('%s', '%s') Failed:GetLastError=%d", nxlFile.c_str(), fileName, error);
									}
								}
								CSwRMXMgr::GetInstance()->RMX_DeleteFile(bakfile.c_str());
							}
						}
					}
				}
				//TBD::always update the active root doc? since the saving file may not be the root file 
				//Reset Overlay
				CSwViewOverlay swOverlayObj(fileName);
				swOverlayObj.ResetViewOverlay();
			}
			break;
		}

	case swFileSaveAsCopy:
	case swFileSaveAsCopyAndOpen:
		{
			wstring srcExt, tarExt;
			tarExt = FileUtils::GetFileExt(fileName);
			if (m_fileName.empty()) {
				long doctype = theSwUtils->GetModelDocType();
				srcExt = theSwUtils->GetModelDocTypeByEnum(doctype);
			}
			else {
				srcExt = FileUtils::GetFileExt(m_fileName);
			}
			if (SwFileTypes::IsSupportedFileFormat(srcExt, tarExt, handlerType))
			{
				LOG_DEBUG_FMT("HandlerType=%d", handlerType);
				if (handlerType == SwFileHandlerMethod::USE_COPY_HOOK) {
					if (
						//saveType == swFileSaveAsCopyAndOpen &&//make independent=4, make virtual is 3 or 4
						!newFileLoaded
						&& !m_isvirtual && !isvirtual) {
						//LOG_DEBUG("Potential make virtual or make independent happing here...");
						if (m_fileProtected)
						{
							bool isprotected;
							if (CSwRMXMgr::GetInstance()->RMX_RPMGetFileStatus(fileName, &isprotected)
								&& !isprotected) {
								LOG_DEBUG_FMT("FileNotProtected('%s'):CopyFile hook may not work well", fileName);
								if (!CSwRMXMgr::GetInstance()->ProtectFile(fileName, m_fileTags)) {
									LOG_DEBUG_FMT("Failed to protect('%s') with tags('%s')", fileName, m_fileTags.c_str());
									ret = false;
								}
								else
								{
									LOG_DEBUG_FMT("Protected('%s') with tags('%s')", fileName, m_fileTags.c_str());
								}
							}
						}
					}
				}
				else if (handlerType == SwFileHandlerMethod::USE_NORMAL) {
					status = FileSaveAsHandler(fileName);
					if (!status) {
						LOG_ERROR_FMT("Save operation could not be handled for fileName = %s ,saveType=%d ", fileName, saveType);
						ret = false;
					}
				}
			}
		}
		break;
	}
	//udpate cached values
	m_isvirtual = isvirtual;
	m_physicalFile = physicalFile;

	return ret;

}

bool CSwDocument::FileSaveAsHandler(BSTR fileName)
{
	LOG_INFO("FileName = " << fileName);
	CSwRMXMgr  * swRMXMgrInstance=CSwRMXMgr::GetInstance();
	wstring fileTag = L"";
	long docType;
	iDocument->GetType(&docType);
	LOG_DEBUG_FMT("[%p]currentName='%s' type=%d isprotected=%d", iDocument.p, m_fileName.c_str(), docType, m_fileProtected);
	if (m_fileName.empty() 
		&& docType == swDocPART
		&& !m_fileProtected)
	{
		LOG_DEBUG_FMT("Skip processing non-protected sldprt model");
		return true;
	}
	bool isTagMerged = swRMXMgrInstance->IsTagMergeRequired(m_fileName, fileName);
	if (isTagMerged) {
		swRMXMgrInstance->DoTagMerge(fileTag);
	}
	else {
		fileTag = m_fileTags;
	}
	if (!fileTag.empty()) {
		LOG_INFO("File Protection Begin :" << fileName);
		LOG_INFO_FMT("SourceFileName=%s , TargetFileName = %s , IsTagMerged = %d", m_fileName.c_str(), fileName, isTagMerged);
		if (!swRMXMgrInstance->ProtectFile(fileName, fileTag)) {
			LOG_ERROR("Failed to protect the file = " << fileName);
			return false;
		}
		LOG_INFO("File Protection End :" << fileName);
	}
	return true;
}

void CSwDocument::AttachModelEventsToAllReferences() {
	VARIANT_BOOL attached = VARIANT_TRUE;
	vector<wstring> referencedFileNames = CSwUtils::GetInstance()->GetReferencedModelDocuments();
	for (auto fileName : referencedFileNames) {
		CComPtr<IModelDoc2> modelDoc;
		CComBSTR bstrFileName((int)fileName.size(), fileName.data());
		iSwApp->IGetOpenDocumentByName2(bstrFileName, &modelDoc);
		attached = userAddin->AttachEventsToModelDocument(modelDoc);
		if (!attached)
			LOG_ERROR("Event listener cannot be added to the model document with FileName = " << fileName);
	}

}

bool CSwDocument::OnModelDocViewNotify(LPDISPATCH swObject) {
	LOG_DEBUG("CSwDocument::OnModelDocViewNotify called");
	if (swObject == NULL)
		return false;

	CComPtr<IModelView> iModelView;
	swObject->QueryInterface(__uuidof(IModelView), reinterpret_cast<void**>(&iModelView));
	VARIANT_BOOL attached = AttachEventsToModelView(iModelView);
	if (!attached)
		return false;
	
	/*
	//Check the iModelView belongs to which model document, get the corresponding file name and apply overlay.
	CComPtr<IModelDoc2> modelDoc = CSwUtils::GetInstance()->GetModeDocForModelView(iModelView);
	if (modelDoc) {
		CComBSTR szPathName;
		modelDoc->GetPathName(&szPathName);
		CSwViewOverlay swOverlayObj(FileUtils::bstr2ws(szPathName));
		swOverlayObj.ResetViewOverlay();
	}
	*/
	return true;
}