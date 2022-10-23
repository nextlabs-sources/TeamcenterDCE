// SldWorksRMX.h : Declaration of the CSldWorksRMX

#pragma once

#include "SldWorksRMX_i.h"

#include "resource.h"       // main symbols
#include <comsvcs.h>
#include <map>
#include <CADRMXTypeDef.h>
#include <vector>

class CSwDocument;
typedef std::map<IUnknown*, CSwDocument*> TMapIUnknownToDocument;

#define ID_SLDWORKS_EVENTS 0

// CSldWorksRMX

#define NXL_FILE_EXT L".nxl"


class FileDetails {
public:
	static vector<FileDetails> ProtectFileList;
	FileDetails(wstring fname, wstring ftags) : fileName(fname),fileTags(ftags) {

	}
	~FileDetails() {

	}
	bool Add(const FileDetails &fd);
	inline wstring GetFileName() {
		return fileName;
	}

	inline wstring GetFileTag() {
		return fileTags;
	}
private:
	wstring fileName;
	wstring fileTags;
};

class CSwCommandHandler {
public:
	CSwCommandHandler(CComPtr<ISldWorks> iswapp) :m_iSwApp(iswapp) {
	}
	virtual ~CSwCommandHandler() {
	}
	virtual void OnAppCommandOpenPreNotify(int command, int userCommand) {
		LOG_DEBUG_FMT(L"command=%d usercommand=%d", command, userCommand);
	}

	virtual void OnAppCommandCloseNotify(int command, int reason) {
		LOG_DEBUG_FMT(L"command=%d reason=%d", command, reason);
	}
protected:
	CComPtr<ISldWorks> m_iSwApp;
};


class ATL_NO_VTABLE CSldWorksRMX :

	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSldWorksRMX, &CLSID_SldWorksRMX>,
	public IDispatchImpl<ISldWorksRMX, &IID_ISldWorksRMX, &LIBID_SldWorksRMXLib, 1, 0>,
	public ISwAddin,
	public IDispEventImpl<ID_SLDWORKS_EVENTS, CSldWorksRMX, &__uuidof(DSldWorksEvents), &LIBID_SldWorks, ID_SLDWORKS_TLB_MAJOR, ID_SLDWORKS_TLB_MINOR>
{
	typedef IDispEventImpl<ID_SLDWORKS_EVENTS, CSldWorksRMX, &__uuidof(DSldWorksEvents), &LIBID_SldWorks, ID_SLDWORKS_TLB_MAJOR, ID_SLDWORKS_TLB_MINOR> CSldWorksEvents;

private:
	CComPtr<ISldWorks>  iSwApp;
	CComPtr<ICommandManager> m_spCmdMgr;
	vector<std::shared_ptr<CSwCommandHandler>> m_commandHandlers;
	
	long addinID;
	long m_swMajNum;
	long m_swMinNum;
	//This mapping will contain references to all open Documents, and ensure 
	//that we do not attempt to attach event handlers to an already opened doc. 
	TMapIUnknownToDocument openDocs;
	wstring m_defaultSaveFolder;
	static const std::vector<pair<int, int>> SwCommandWndHookList;
	bool m_IsWndHookStarted;
	wstring m_rmxRootDir;

public:
	static vector<wstring> RPMDirList;
	static DWORD m_osCurrMajorVer;
	static vector<pair<wstring, wstring>> TCDestSrcDocMap;

public:
	CSldWorksRMX():m_IsWndHookStarted(false)
	{

	}

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_SldWorksRMX)

	DECLARE_NOT_AGGREGATABLE(CSldWorksRMX)

	BEGIN_COM_MAP(CSldWorksRMX)
		COM_INTERFACE_ENTRY(ISldWorksRMX)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(ISwAddin)
	END_COM_MAP()


	// ISwAddin4


	// ISwAddin Methods
public:
	CComPtr<ISldWorks>  GetSldWorksPtr() { return iSwApp != NULL ? iSwApp : NULL; }
	//These methods will connect and disconnect this addin to the SolidWorks events
	VARIANT_BOOL AttachEventHandlers();
	VARIANT_BOOL DetachEventHandlers();
	//These methods will connect and disconnect this addin to the SolidWorks Model events
	VARIANT_BOOL AttachModelEventHandler(CComPtr<IModelDoc2> iModelDoc2);
	VARIANT_BOOL DetachModelEventHandler(IUnknown *iUnk);
	HRESULT AttachEventsToAllDocuments();
	VARIANT_BOOL AttachEventsToModelDocument(CComPtr<IModelDoc2> iModelDoc2);
	TMapIUnknownToDocument OpenDocumentsTable() { return openDocs; }
	CSwDocument* FindDocumentInfo(CComPtr<IModelDoc2>& modelDoc);

	int GetSldWorksTlbMajor() { return (m_swMajNum >= ID_SLDWORKS_TLB_MAJOR) ? m_swMajNum : 0; }
	int GetSldWorksTlbMinor() { return m_swMinNum; }
	bool Init();
	void AddCommandMgr();
	void RemoveCommandMgr();
	bool CompareIDs(long * storedIDs, long storedSize, long * addinIDs, long addinSize);
	BSTR GetCurrentFile();
	static void ProcessRecords();
	void AddRPMFolder(BSTR fileName);
	inline wstring GetDefaultSaveFolder() {
		return m_defaultSaveFolder;
	}

	static void AddToTCDestSrcDocMap(pair<wstring,wstring> pairObj){
		TCDestSrcDocMap.push_back(pairObj);
	}

	static void ClearTCDestSrcDocMap() {
		TCDestSrcDocMap.clear();
	}

	inline bool IsSwCommandUsingWndHook(int command, int userCommand) {
		return std::find(SwCommandWndHookList.begin(), SwCommandWndHookList.end(), make_pair(command, userCommand)) != SwCommandWndHookList.end() ? true : false;
	}

	//Event Handlers
	//These are the methods that are called when certain SolidWorks events are fired
	STDMETHOD(OnDocChange)(void);
	//STDMETHOD(OnModelDocChange)(void);
	STDMETHOD(OnDocLoad)(BSTR docTitle, BSTR docPath);
	STDMETHOD(OnFileNew)(LPDISPATCH newDoc, long docType, BSTR templateName);
	STDMETHOD(OnFileClose)(BSTR fileName, long closeReason);
	STDMETHOD(OnFileOpenPostNotify)(BSTR fileName);
	STDMETHOD(OnFileOpenPreNotify)(BSTR fileName);
	//STDMETHOD(OnFileOpenNotify)(BSTR fileName);
	//STDMETHOD(OnFileNewPreNotify)(int docType, BSTR templateName);
	STDMETHOD(OnAppCommandCloseNotify)(int command, int reason);
	//STDMETHOD(OnAppNonNativeFileOpenNotify)(BSTR fileName);
	STDMETHOD(OnAppBeginTranslationNotify)(BSTR fileName, int options);
	//STDMETHOD(OnAppEndTranslationNotify)(BSTR fileName, int options);
	STDMETHOD(OnAppCommandOpenPreNotify)(int command,int userCommand);
	STDMETHOD(OnAppReferencedFilePreNotify)(BSTR fileName, int fileStatus);
	


	BEGIN_SINK_MAP(CSldWorksRMX)
		SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppActiveDocChangeNotify, OnDocChange)
		SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppDocumentLoadNotify2, OnDocLoad)
		SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppFileNewNotify2, OnFileNew)
		SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppFileCloseNotify, OnFileClose)
		//SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppActiveModelDocChangeNotify, OnModelDocChange)
		SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppFileOpenPostNotify, OnFileOpenPostNotify)
		SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppFileOpenPreNotify, OnFileOpenPreNotify)
		//SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppFileOpenNotify2, OnFileOpenNotify)
		//SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppFileNewPreNotify, OnFileNewPreNotify)
		SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppCommandCloseNotify, OnAppCommandCloseNotify)
		SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppCommandOpenPreNotify, OnAppCommandOpenPreNotify)
		//SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppNonNativeFileOpenNotify, OnAppNonNativeFileOpenNotify)
		SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppBeginTranslationNotify, OnAppBeginTranslationNotify)
		//SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppEndTranslationNotify, OnAppEndTranslationNotify)	
		SINK_ENTRY_EX(ID_SLDWORKS_EVENTS, __uuidof(DSldWorksEvents), swAppReferencedFilePreNotify2, OnAppReferencedFilePreNotify)
	END_SINK_MAP()

	// ISwAddin Methods
	//These are the methods inherited from the ISwAddin interface that 
	//need to be implemented in order to connect the addin to SolidWorks
public:
	STDMETHOD(ConnectToSW)(LPDISPATCH ThisSW, long Cookie, VARIANT_BOOL * IsConnected);
	STDMETHOD(DisconnectFromSW)(VARIANT_BOOL * IsDisconnected);

	// Command callbacks
	STDMETHOD(OnRMXProtect)(void);
	STDMETHOD(OnRMXProtectEnable)(long* status);
	STDMETHOD(OnRMXViewInfo)(void);
	STDMETHOD(OnRMXViewInfoEnable)(long* status);
	STDMETHOD(OnRMXProtectWithRefs)(void);
	STDMETHOD(OnRMXProtectWithRefsEnable)(long* status);
	STDMETHOD(OnRMXAbout)(void);

	bool IsSwimAddinActive();
private:
	bool IsRMXCommand(long cmdID);
	void InitRPMFolders();

};

OBJECT_ENTRY_AUTO(__uuidof(SldWorksRMX), CSldWorksRMX)

