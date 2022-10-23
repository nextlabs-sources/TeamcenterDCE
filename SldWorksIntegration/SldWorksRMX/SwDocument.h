// SwDocument.h : Declaration of the CSwDocument

#pragma once

#include "SldWorksRMX.h"
#include "resource.h"       // main symbols
//Defines
#define ID_PART_EVENTS 1
#define ID_ASSEMBLY_EVENTS 2
#define ID_DRAWING_EVENTS 3
class CSwAddin4;

#include <map>
class CDocView;
typedef std::map<IUnknown*, CDocView*> TMapIUnknownToModelView;

// CSwDocument

class ATL_NO_VTABLE CSwDocument :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSwDocument, &CLSID_SwDocument>,
	public IDispatchImpl<ISwDocument, &IID_ISwDocument, &LIBID_SldWorksRMXLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDispEventImpl<ID_PART_EVENTS, CSwDocument, &__uuidof(DPartDocEvents), &LIBID_SldWorks, ID_SLDWORKS_TLB_MAJOR, ID_SLDWORKS_TLB_MINOR>,
	public IDispEventImpl<ID_ASSEMBLY_EVENTS, CSwDocument, &__uuidof(DAssemblyDocEvents), &LIBID_SldWorks, ID_SLDWORKS_TLB_MAJOR, ID_SLDWORKS_TLB_MINOR>,
	public IDispEventImpl<ID_DRAWING_EVENTS, CSwDocument, &__uuidof(DDrawingDocEvents), &LIBID_SldWorks, ID_SLDWORKS_TLB_MAJOR, ID_SLDWORKS_TLB_MINOR>

{

	typedef IDispEventImpl<ID_PART_EVENTS, CSwDocument, &__uuidof(DPartDocEvents), &LIBID_SldWorks, ID_SLDWORKS_TLB_MAJOR, ID_SLDWORKS_TLB_MINOR> CPartEvents;
	typedef IDispEventImpl<ID_ASSEMBLY_EVENTS, CSwDocument, &__uuidof(DAssemblyDocEvents), &LIBID_SldWorks, ID_SLDWORKS_TLB_MAJOR, ID_SLDWORKS_TLB_MINOR> CAssemblyEvents;
	typedef IDispEventImpl<ID_DRAWING_EVENTS, CSwDocument, &__uuidof(DDrawingDocEvents), &LIBID_SldWorks, ID_SLDWORKS_TLB_MAJOR, ID_SLDWORKS_TLB_MINOR> CDrawingEvents;

private:
	long type;
	CComPtr<IModelDoc2> iDocument;
	CComPtr<ISldWorks> iSwApp;
	CSldWorksRMX* userAddin;
	TMapIUnknownToModelView openModelViews;

	wstring m_fileName;
	std::vector<int> m_fileRights;
	std::wstring m_fileTags;
	bool m_fileProtected;
	bool m_isvirtual = false;
	std::wstring m_physicalFile;
	bool m_denyNotified = false;

public:
	bool CachedIsVirtual() const { return m_isvirtual; }
	bool CachedIsProtected() const { return m_fileProtected; }
	const wstring& CachedFileTags() const { return m_fileTags; }
	const wstring& CachedFileName() const { return m_fileName; }
	const wstring& CachedPhysicalFile() const { return m_physicalFile; }
	bool CanSaveVirtual(const wstring&fileName, bool silent=false) const;

	CSwDocument();

	~CSwDocument();

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_SWDOCUMENT)

	DECLARE_NOT_AGGREGATABLE(CSwDocument)

	BEGIN_COM_MAP(CSwDocument)
		COM_INTERFACE_ENTRY(ISwDocument)
		COM_INTERFACE_ENTRY(IDispatch)
	END_COM_MAP()




	// ISwDocument
public:	 
	void Init(CSldWorksRMX* addinPtr, IModelDoc2* modDoc);
	long GetType();
	bool PostFileSaveHandler(int saveType, BSTR  fileName);
	bool FileSaveAsHandler(BSTR fileName);
	VARIANT_BOOL AttachEventHandlers(void);
	VARIANT_BOOL AttachModelViewEventHandlers(void);
	VARIANT_BOOL AttachEventsToModelView(CComPtr<IModelView> iModelView);
	VARIANT_BOOL DetachEventHandlers(void);
	VARIANT_BOOL DetachModelViewEventHandler(IUnknown *IUnk);
	HRESULT ComponentStateChange(LPDISPATCH componentModel, long newCompState);
	void AttachModelEventsToAllReferences();
	bool OnModelDocViewNotify(LPDISPATCH swObject);
	
	//Event Handlers
	STDMETHOD(OnPartDestroy)(int destroyType);
	STDMETHOD(OnPartFileModify)(void);
	STDMETHOD(OnPartFileSaveAs)(BSTR fileName);
	STDMETHOD(OnPartFileSave)(BSTR fileName);
	STDMETHOD(OnPartFilePostSave)(int saveType, BSTR  fileName);
	STDMETHOD(OnPartFileSavePostCancelNotify)(void);
	STDMETHOD(OnPartViewNewNotify)(LPDISPATCH swObject);
	STDMETHOD(OnAssemblyDestroy)(int destroyType);
	STDMETHOD(OnAssemblyComponentStateChangeNotify2)(LPDISPATCH componentModel, BSTR compName, long oldCompState, long newCompState);
	STDMETHOD(OnAssemblyComponentStateChangeNotify)(LPDISPATCH componentModel, long oldCompState, long newCompState);
	STDMETHOD(OnAssemblyComponentVisualPropertiesChangeNotify)(LPDISPATCH swObject);
	STDMETHOD(OnAssemblyComponentDisplayStateChangeNotify)(LPDISPATCH swObject);
	STDMETHOD(OnAssemblyFileSaveAs)(BSTR fileName);
	STDMETHOD(OnAssemblyFileSave)(BSTR fileName);
	STDMETHOD(OnAssemblyFilePostSave)(int saveType, BSTR  fileName);
	STDMETHOD(OnAssemblyFileModify)(void);
	STDMETHOD(OnAssemblyFileSavePostCancelNotify)(void);
	STDMETHOD(OnAssemblyAddItemNotify)(int entityType, BSTR itemName);
	STDMETHOD(OnAssemblyViewNewNotify)(LPDISPATCH swObject);
	STDMETHOD(OnDrawingFileModify)(void);
	STDMETHOD(OnDrawingDestroy)(int destroyType);
	STDMETHOD(OnDrawingFilePostSave)(int saveType, BSTR  fileName);
	STDMETHOD(OnDrawingFileSave)(BSTR fileName);
	STDMETHOD(OnDrawingFileSaveAs)(BSTR fileName);	
	STDMETHOD(OnDrawingAddItemNotify)(int entityType, BSTR itemName);
	STDMETHOD(OnDrawingViewNewNotify)(LPDISPATCH swObject);


	//Event Sinks
	//The SINK_MAP connects the specified Model event to a specific event handler
	BEGIN_SINK_MAP(CSwDocument)
		SINK_ENTRY_EX(ID_PART_EVENTS, __uuidof(DPartDocEvents), swPartDestroyNotify2, OnPartDestroy)
		SINK_ENTRY_EX(ID_PART_EVENTS, __uuidof(DPartDocEvents), swPartFileSaveNotify, OnPartFileSave)
		SINK_ENTRY_EX(ID_PART_EVENTS, __uuidof(DPartDocEvents), swPartFileSaveAsNotify2, OnPartFileSaveAs)
		SINK_ENTRY_EX(ID_PART_EVENTS, __uuidof(DPartDocEvents), swPartFileSavePostNotify, OnPartFilePostSave)
		SINK_ENTRY_EX(ID_PART_EVENTS, __uuidof(DPartDocEvents), swPartModifyNotify, OnPartFileModify)
		SINK_ENTRY_EX(ID_PART_EVENTS, __uuidof(DPartDocEvents), swPartViewNewNotify2, OnPartViewNewNotify)
		SINK_ENTRY_EX(ID_PART_EVENTS, __uuidof(DPartDocEvents), swPartFileSavePostCancelNotify, OnPartFileSavePostCancelNotify)
		SINK_ENTRY_EX(ID_ASSEMBLY_EVENTS, __uuidof(DAssemblyDocEvents), swAssemblyDestroyNotify2, OnAssemblyDestroy)
		SINK_ENTRY_EX(ID_ASSEMBLY_EVENTS, __uuidof(DAssemblyDocEvents), swAssemblyComponentStateChangeNotify2, OnAssemblyComponentStateChangeNotify2)
		SINK_ENTRY_EX(ID_ASSEMBLY_EVENTS, __uuidof(DAssemblyDocEvents), swAssemblyComponentStateChangeNotify, OnAssemblyComponentStateChangeNotify)
		SINK_ENTRY_EX(ID_ASSEMBLY_EVENTS, __uuidof(DAssemblyDocEvents), swAssemblyComponentVisualPropertiesChangeNotify, OnAssemblyComponentVisualPropertiesChangeNotify)
		SINK_ENTRY_EX(ID_ASSEMBLY_EVENTS, __uuidof(DAssemblyDocEvents), swAssemblyComponentDisplayStateChangeNotify, OnAssemblyComponentDisplayStateChangeNotify)
		SINK_ENTRY_EX(ID_ASSEMBLY_EVENTS, __uuidof(DAssemblyDocEvents), swAssemblyModifyNotify, OnAssemblyFileModify)
		SINK_ENTRY_EX(ID_ASSEMBLY_EVENTS, __uuidof(DAssemblyDocEvents), swAssemblyFileSaveNotify, OnAssemblyFileSave)
		SINK_ENTRY_EX(ID_ASSEMBLY_EVENTS, __uuidof(DAssemblyDocEvents), swAssemblyFileSaveAsNotify2, OnAssemblyFileSaveAs)
		SINK_ENTRY_EX(ID_ASSEMBLY_EVENTS, __uuidof(DAssemblyDocEvents), swAssemblyFileSavePostNotify, OnAssemblyFilePostSave)
		SINK_ENTRY_EX(ID_ASSEMBLY_EVENTS, __uuidof(DAssemblyDocEvents), swAssemblyFileSavePostCancelNotify, OnAssemblyFileSavePostCancelNotify)
		SINK_ENTRY_EX(ID_ASSEMBLY_EVENTS, __uuidof(DAssemblyDocEvents), swAssemblyAddItemNotify, OnAssemblyAddItemNotify)
		SINK_ENTRY_EX(ID_ASSEMBLY_EVENTS, __uuidof(DAssemblyDocEvents), swAssemblyViewNewNotify2, OnAssemblyViewNewNotify)
		SINK_ENTRY_EX(ID_DRAWING_EVENTS, __uuidof(DDrawingDocEvents), swDrawingModifyNotify, OnDrawingFileModify)
		SINK_ENTRY_EX(ID_DRAWING_EVENTS, __uuidof(DDrawingDocEvents), swDrawingDestroyNotify2, OnDrawingDestroy)
		SINK_ENTRY_EX(ID_DRAWING_EVENTS, __uuidof(DDrawingDocEvents), swDrawingFileSaveNotify, OnDrawingFileSave)
		SINK_ENTRY_EX(ID_DRAWING_EVENTS, __uuidof(DDrawingDocEvents), swDrawingFileSaveAsNotify2, OnDrawingFileSaveAs)
		SINK_ENTRY_EX(ID_DRAWING_EVENTS, __uuidof(DDrawingDocEvents), swDrawingFileSavePostNotify, OnDrawingFilePostSave)		
		SINK_ENTRY_EX(ID_DRAWING_EVENTS, __uuidof(DDrawingDocEvents), swDrawingAddItemNotify, OnDrawingAddItemNotify)
		SINK_ENTRY_EX(ID_DRAWING_EVENTS, __uuidof(DDrawingDocEvents), swDrawingViewNewNotify2, OnDrawingViewNewNotify)
	
	END_SINK_MAP()
};

OBJECT_ENTRY_AUTO(__uuidof(SwDocument), CSwDocument)
