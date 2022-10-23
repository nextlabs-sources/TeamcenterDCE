// SldWorksRMX.cpp : Implementation of CSldWorksRMX

#include "stdafx.h"
#include <sysinfoapi.h>
#include <atlsafe.h>
#include <PathEx.h>

#include "SldWorksRMX.h"
#include "SwDocument.h"
#include <string>
#include <unordered_map>
#include <list>
#include "SwRMXMgr.h"
#include "FileUtils.h"
#include "SwHooks.h"
#include "SwUserAuthorization.h"
#include "SwUtils.h"
#include "SwRMXServerSocket.h"
#include "SwRMXSwimInterface.h"
#include "SwRMXDialog.h"
#include "SwRMXClientSocket.h"
#include "RegistryHelper.h"
#include "SwViewOverlay.h"
#include "CommonUtils.h"
#include "SwWndHooks.h"
#include "SwimUtils.h"


#define REG_KEY_WINCURRVER L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"
#define REG_VALUE_WINMAJORVER L"CurrentMajorVersionNumber"

#define RMX_CMD_GROUP_ID 90
#define RMX_CMD_ID_BASE 900

// CSldWorksRMX
vector<FileDetails> FileDetails::ProtectFileList = {};
vector<wstring> CSldWorksRMX::RPMDirList = {};
DWORD CSldWorksRMX::m_osCurrMajorVer = 0;
vector<pair<wstring, wstring>> CSldWorksRMX::TCDestSrcDocMap = {};
const vector<pair<SwCommand, SwUserCommand>> CSldWorksRMX::SwCommandWndHookList = {
	{ make_pair(swCommands_File_Find,0)},
	{ make_pair(swCommands_File_Copy_Design,0)}
	//{ make_pair(swCommands_User_Toolbar_Min,swUserCommands_JtExportCommand)}
};

// Type of RMX command
struct sRMXCMD
{
	UINT nTipID;
	UINT nHintID;
	long lImageIndex;
	BSTR bstrExecuteCallback;
	BSTR bstrEnableCallback;
	long lUserID;
	long lCmdIndex;
	long lCmdID;
};

#define CASE_TRUE(cmd) \
case cmd:	\
	LOG_DEBUG_FMT(L"%d=%S", cmd, #cmd);	\
	return true;

class COverlayUpdater:public CSwCommandHandler {
public:
	COverlayUpdater(CComPtr<ISldWorks> iswapp) :CSwCommandHandler(iswapp) {
		LOG_DEBUG_FMT(L"iswapp=%p m_iSwApp=%p", iswapp, m_iSwApp);
	}
	bool CanHandle(int command) {
		if (m_iSwApp!=nullptr)
		{
			switch (command) {
				CASE_TRUE(swCommands_MakeDrawingFromPartAssembly);
				CASE_TRUE(swCommands_MakeAssemblyFromPartAssembly);
				CASE_TRUE(swCommands_New);
				CASE_TRUE(swCommands_InsertComponents);
			default:
				return false;
			}
		}
		return false;
	}
	void OnAppCommandCloseNotify(int command, int reason) override{
		if (!CanHandle(command))
			return;
		LOG_DEBUG(L"RefreshingOverlay...");
		CSwViewOverlay swOverlayObj(L"");
		swOverlayObj.ResetViewOverlay();
	}
};

// Global list of RMX commands
std::vector<sRMXCMD> g_rmxCmds = {
		sRMXCMD{IDS_CMD_PROTECT_TIP, IDS_CMD_PROTECT_HINT, 0, L"OnRMXProtect", L"OnRMXProtectEnable", RMX_CMD_ID_BASE, -1, -1},
		sRMXCMD{IDS_CMD_PROTECTREFS_TIP, IDS_CMD_PROTECTREFS_HINT, 1, L"OnRMXProtectWithRefs", L"OnRMXProtectWithRefsEnable", RMX_CMD_ID_BASE + 1, -1, -1},
		sRMXCMD{IDS_CMD_VIEWINFO_TIP, IDS_CMD_VIEWINFO_HINT, 2, L"OnRMXViewInfo", L"OnRMXViewInfoEnable", RMX_CMD_ID_BASE + 2, -1, -1},
		sRMXCMD{IDS_CMD_ABOUT_TIP, IDS_CMD_ABOUT_HINT, 3, L"OnRMXAbout", L"", RMX_CMD_ID_BASE + 3, -1, -1}
};

bool FileDetails::Add(const FileDetails &fd) {
	for (auto fp : ProtectFileList) {
		if (!_wcsicmp(fp.fileName.c_str(),fd.fileName.c_str()))
			return false;
	}
	ProtectFileList.push_back(fd);
	return true;
}

bool CSldWorksRMX::CompareIDs(long * storedIDs, long storedSize, long * addinIDs, long addinSize)
{
	std::list<long> storedList;
	std::list<long> addinList;

	for (int i = 0; i < storedSize; i++)
	{
		storedList.push_front(storedIDs[i]);
	}
	for (int j = 0; j < addinSize; j++)
	{
		addinList.push_front(addinIDs[j]);
	}

	addinList.sort();
	storedList.sort();

	if (addinList.size() != storedList.size())
	{
		return false;
	}
	else
	{
		std::list<long>::iterator al = addinList.begin();
		//Iterate through the two lists and make sure they match
		for (std::list<long>::iterator sl = storedList.begin(); sl != storedList.end(); sl++)
		{
			if (*al != *sl)
			{
				return false;
			}
			al++;
		}
	}
	return true;
}

STDMETHODIMP CSldWorksRMX::OnDocChange(void)
{
	LOG_DEBUG(L"CSldWorksRMX::OnDocChange called");
	wstring fileName = CSwUtils::GetInstance()->GetCurrentActiveFile();
	CSwViewOverlay swOverlayObj(fileName);
	swOverlayObj.ResetViewOverlay();
	
	return S_OK;
}

//Called after a document is opened
STDMETHODIMP CSldWorksRMX::OnFileOpenPostNotify(BSTR fileName)
{
	LOG_DEBUG(L"FileName = " << fileName);
	HRESULT hres = S_OK;
	hres = AttachEventsToAllDocuments();
	return hres;
}

//Called when a new document is created or a document is loaded
STDMETHODIMP CSldWorksRMX::OnDocLoad(BSTR docTitle, BSTR docPath)
{
	LOG_DEBUG_FMT(L"DocTitle = %s, DocPath = %s", docTitle, docPath) ;
	auto modelDoc = theSwUtils->GetModelDocByFileName(docPath);
	LOG_DEBUG_FMT(L"ModelDoc=%p", modelDoc.p);
	AttachEventsToModelDocument(modelDoc);
	return S_OK;
}

VARIANT_BOOL CSldWorksRMX::AttachEventHandlers()
{
	VARIANT_BOOL attached = VARIANT_TRUE;
	this->m_libid = LIBID_SldWorks;
	this->m_wMajorVerNum = GetSldWorksTlbMajor();
	this->m_wMinorVerNum = 0;

	CSldWorksEvents::_tih.m_wMajor = this->m_wMajorVerNum;

	// Connect to the SldWorks event sink
	HRESULT success = this->DispEventAdvise(iSwApp, &__uuidof(DSldWorksEvents));

	if (success != S_OK)
		return VARIANT_FALSE;

	// Connect event handlers to all previously open documents
	success = AttachEventsToAllDocuments();
	if (success != S_OK)
		attached = VARIANT_FALSE;
	return attached;
}

HRESULT CSldWorksRMX::AttachEventsToAllDocuments()
{
	VARIANT_BOOL attached = VARIANT_TRUE;
	USES_CONVERSION;
	CComPtr<IModelDoc2> iModelDoc2;
	iSwApp->IGetFirstDocument2(&iModelDoc2);

	while (iModelDoc2 != NULL)
	{
		attached = AttachEventsToModelDocument(iModelDoc2);
		if(!attached)
			return E_FAIL;

		CComPtr<IModelDoc2> iNextModelDoc2;
		iModelDoc2->IGetNext(&iNextModelDoc2);
		iModelDoc2.Release();
		iModelDoc2 = iNextModelDoc2;
	}
	return S_OK;
}
CSwDocument* CSldWorksRMX::FindDocumentInfo(CComPtr<IModelDoc2> &modelDoc) {
	if (modelDoc) {
		TMapIUnknownToDocument::iterator iter;
		iter = openDocs.find(modelDoc);
		if (iter != openDocs.end()
			&& iter->second != nullptr) {
			LOG_DEBUG_FMT("[%p]'%s'", modelDoc.p, iter->second->CachedFileName().c_str());
			return iter->second;
		}
	}
	LOG_ERROR_FMT("[%p]no CSwDocument is found", modelDoc.p);
	return nullptr;
}

VARIANT_BOOL CSldWorksRMX::AttachEventsToModelDocument(CComPtr<IModelDoc2> iModelDoc2) {
	VARIANT_BOOL attached = VARIANT_TRUE;
	if (iModelDoc2 == NULL)
		return VARIANT_FALSE;
	
	CComBSTR pathName;
	iModelDoc2->GetPathName(&pathName);
	
	TMapIUnknownToDocument::iterator iter;
	iter = openDocs.find(iModelDoc2);
	if (iter == openDocs.end())
	{
		LOG_WARN_STR(L"CSldWorksRMX::DocumentLoadNotify current part not found");
		attached = AttachModelEventHandler(iModelDoc2);
		if (!attached)
			return VARIANT_FALSE;
	}

	else
	{
		VARIANT_BOOL connected = VARIANT_FALSE;
		CComObject<CSwDocument> *docHandler = NULL;
		docHandler = (CComObject<CSwDocument>*)openDocs[iModelDoc2];

		if (docHandler != NULL)
		{
			connected = docHandler->AttachModelViewEventHandlers();
		}
	}

	return attached;
}

//Create an event handling object for this document and listen for the model's events
VARIANT_BOOL CSldWorksRMX::AttachModelEventHandler(CComPtr<IModelDoc2> iModelDoc2)
{
	VARIANT_BOOL attached = VARIANT_TRUE;
	if (iModelDoc2 == NULL)
		return VARIANT_FALSE;

	TMapIUnknownToDocument::iterator iter;
	iter = openDocs.find(iModelDoc2);
	if (iter == openDocs.end())
	{
		//Cretae a new Document event handler
		CComObject<CSwDocument> *pDoc;
		CComObject<CSwDocument>::CreateInstance(&pDoc);
		pDoc->Init((CComObject<CSldWorksRMX>*)this, iModelDoc2);

		//Listen for the doc's events
		attached = pDoc->AttachEventHandlers();
		if (!attached)
			return VARIANT_FALSE;

		//Add it to the map
		openDocs.insert(TMapIUnknownToDocument::value_type(iModelDoc2, pDoc));
		iModelDoc2.p->AddRef();
		pDoc->AddRef();
	}
	return attached;
}

//Stop listening for SolidWorks Events
VARIANT_BOOL CSldWorksRMX::DetachEventHandlers()
{
	VARIANT_BOOL detached = VARIANT_TRUE;

	// Disconnect from the SldWorks event sink
	HRESULT success = this->DispEventUnadvise(iSwApp, &__uuidof(DSldWorksEvents));

	CSldWorksEvents::_tih.m_plibid = &GUID_NULL;

	if (success != S_OK)
		return VARIANT_FALSE;

	// Disconnect all event handlers
	TMapIUnknownToDocument::iterator iter;
	iter = openDocs.begin();

	for (iter = openDocs.begin(); iter != openDocs.end(); /*iter++*/) //The iteration is performed inside the loop
	{
		TMapIUnknownToDocument::value_type obj = *iter;
		iter++;

		CComObject<CSwDocument> *pDoc = (CComObject<CSwDocument>*)obj.second;
		detached = pDoc->DetachEventHandlers();
	}
	return detached;
}

//Stop listening for events on the specified model
VARIANT_BOOL CSldWorksRMX::DetachModelEventHandler(IUnknown *iUnk)
{
	VARIANT_BOOL detached = VARIANT_TRUE;
	TMapIUnknownToDocument::iterator iter;

	iter = openDocs.find(iUnk);
	if (iter != openDocs.end())
	{
		TMapIUnknownToDocument::value_type obj = *iter;
		obj.first->Release();

		CComObject<CSwDocument> *pDoc = (CComObject<CSwDocument>*)obj.second;
		pDoc->Release();

		openDocs.erase(iter);
	}
	return detached;
}

//Called when the active model document changes in SolidWorks
//STDMETHODIMP CSldWorksRMX::OnModelDocChange(void)
//{
//	// TODO: Add your implementation code here
//	LOG_INFO("CSldWorksRMX::On Model Doc Change called");
//	return S_OK;
//}

//Post-notifies the user program when a new file is created.
STDMETHODIMP CSldWorksRMX::OnFileNew(LPDISPATCH newDoc, long docType, BSTR templateName)
{
	LOG_DEBUG_STR(L"CSldWorksRMX::OnFileNew called");
	HRESULT hres = S_OK;
	hres = AttachEventsToAllDocuments();
	return hres;
}

//Fired before a new document is created either using the SOLIDWORKS API or the SOLIDWORKS user-interface.
//STDMETHODIMP CSldWorksRMX::OnFileNewPreNotify(int docType, BSTR templateName)
//{
//	LOG_INFO("CSldWorksRMX::OnFileNewPreNotify called");
//	return S_OK;
//}

STDMETHODIMP CSldWorksRMX::ConnectToSW(LPDISPATCH ThisSW, long Cookie, VARIANT_BOOL * IsConnected)
{
	HMODULE handle = GetModuleHandle(L"SldWorksRMX.dll");
	if (handle != NULL) {
		WCHAR szModuleFilePath[MAX_PATH];
		GetModuleFileNameW(handle, szModuleFilePath, MAX_PATH);
		m_rmxRootDir = FileUtils::GetParentPath(szModuleFilePath);
		wstring rmxRootEnv = L"SLDWORKS_RMX_ROOT=" + m_rmxRootDir;
		if (_wputenv(rmxRootEnv.c_str()) != 0) {
			DBGLOG(L"[Error]Failed to set the environment variable SLDWORKS_RMX_ROOT");
			return S_FALSE;
		}
	}
	else {
		DBGLOG(L"[Error]Failed to get the module handle for SldWorksRMX.dll");
		return S_FALSE;
	}

	//Initialize logging utility
	RMXLOG_BEGIN();
	LOG_INFO(L"CSldWorksRMX::ConnectToSW");

	ThisSW->QueryInterface(__uuidof(ISldWorks), (void**)&iSwApp);
	addinID = Cookie;
	iSwApp->GetCommandManager(Cookie, &m_spCmdMgr);

	//Initialize RMX infrastructure objects
	if (!Init())
		return S_FALSE;

	VARIANT_BOOL status = VARIANT_FALSE;

	iSwApp->SetAddinCallbackInfo((long)_AtlBaseModule.GetModuleInstance(), static_cast<ISldWorksRMX*>(this), addinID, &status);
	//Get the current type library version.
	{
		USES_CONVERSION;
		CComBSTR bstrNum;
		std::string strNum;
		char *buffer;

		iSwApp->RevisionNumber(&bstrNum);

		strNum = W2A(bstrNum);
		m_swMajNum = strtol(strNum.c_str(), &buffer, 10);

		m_swMinNum = 0;
	}

	// Setup the Command Manager
	AddCommandMgr();

	//Listen for events
	*IsConnected = AttachEventHandlers();
	*IsConnected = VARIANT_TRUE;
	return S_OK;
}

bool CSldWorksRMX::Init() {
	
	//Initialize SwUtils Control
	CSwUtils::Init((CComObject<CSldWorksRMX>*)this);

	//Initialize CSwRMXMgr
	bool isRMXInitialized = false;
	CSwRMXMgr *swRMXMgrInstance = CSwRMXMgr::GetInstance();
	
	if (swRMXMgrInstance != nullptr) {
		isRMXInitialized = swRMXMgrInstance->Init();
	}

	if (!isRMXInitialized) {
		return false;
	}
	
	//Key CurrentMajorVersion is added from Windows 10 and not available in prior versions of Windows.
	RegistryHelper::GetLongValFromReg(HKEY_LOCAL_MACHINE, REG_KEY_WINCURRVER,REG_VALUE_WINMAJORVER,m_osCurrMajorVer);

	
	//Initialize Hook
	HookInitializer::Init((CComObject<CSldWorksRMX>*)this);
	HookInitializer::Startup();


	if (!swRMXMgrInstance->SetProcessAsTrusted()) {
		return false;
	}

	//Initialize SwRMXSwimInterface
	CSwRMXSwimInterface::Init((CComObject<CSldWorksRMX>*)this);

	//Create a new thread in which socket will listen for incoming client connections
	HANDLE hnd = (HANDLE)_beginthreadex(0, 0, &CSwRMXServerSocket::InitServerSocket, 0, 0, 0);

	//
	CSwRMXClientSocket clientSocket;
	if (!clientSocket.InitSocket()) {
		clientSocket.Send("Hello\r\n");
		string recvData;
		if (clientSocket.Receive(recvData) > 0) {
			LOG_INFO("Bytes Received = " << recvData.c_str());
		}
	}

	CSwUtils *swUtilsInstance = CSwUtils::GetInstance();

	//Read the default save folder.Setting default save folder is required to support the saving of new assembly/drawing file without launching the dialog
	//when it contains one of the references which doesn't allow 'Save As' operation.
	wstring defaultSaveLoc;
	swUtilsInstance->GetSwUserPreferenceStringValue(swFileLocationsDefaultSave, defaultSaveLoc);
	LOG_INFO("Read preference for swFileLocationsDefaultSave = " << defaultSaveLoc);
	if ( !defaultSaveLoc.empty() && FileUtils::IsDir(defaultSaveLoc)) {
		m_defaultSaveFolder = defaultSaveLoc;
	}
	else {
		
		m_defaultSaveFolder = L"C:";
	}
	
	InitRPMFolders();

	LOG_INFO("Initalizing RMX support for swim");
	SwimUtils::InitSwim();

	//Make java.exe corresponding to swim as trusted process
	/*if (!SwimUtils::SetSwimPIDAsTrusted()) {
		LOG_WARN("Process Id of java.exe corresponding to SWIM couldn't be set as trusted");
	}
	*/

	LOG_INFO("Initalizing RMX CommandHandler");
	m_commandHandlers.emplace_back(new COverlayUpdater(GetSldWorksPtr()));
	return true;
}

STDMETHODIMP CSldWorksRMX::DisconnectFromSW(VARIANT_BOOL * IsDisconnected)
{
	LOG_INFO("CSldWorksRMX::DisconnectFromSW");
	
	RemoveCommandMgr();

	HookInitializer::Shutdown();
	CSwWndHook::UnhookJTExport();
	for (auto fileDir : RPMDirList) {
		CSwRMXMgr::GetInstance()->RMX_RemoveRPMDir(fileDir);
	}
	RPMDirList.clear();
	FileDetails::ProtectFileList.clear();
	TCDestSrcDocMap.clear();
	//Stop listening for events
	*IsDisconnected = DetachEventHandlers();
	
	CSwUserAuthorization::ReleaseInstance();
	CSwViewOverlay::CleanUp();
	CSwUtils::ReleaseInstance();
	
	CSwRMXClientSocket clientSocket;
	if (!clientSocket.InitSocket()) {
		clientSocket.Send("Bye\r\n");
		string recvData;
		if (clientSocket.Receive(recvData) > 0) {
			LOG_INFO("Bytes Received = " << recvData.c_str());
		}
		clientSocket.Close();
	}
	CSwRMXServerSocket::shutDownSocket();
	CSwRMXMgr::ReleaseInstance();
	

	//Make sure you release the SolidWorks pointer last
	iSwApp.Release();
	
	RMXLOG_END();

	return S_OK;
}

void CSldWorksRMX::AddCommandMgr()
{
	LOG_INFO(L"Starting to add command manager...");
	if (!m_spCmdMgr)
	{
		LOG_ERROR(L"Invalid command manager pointer");
		return;
	}

	// Check the old commands in case of conflict or migration needed.
	bool ignorePrevious = true;
	VARIANT_BOOL getDataResult = VARIANT_FALSE;
	long count = 0;
	m_spCmdMgr->GetCommandIDsCount(RMX_CMD_GROUP_ID, &count);
	long* registryIDs = new long[count];
	HRESULT retVal = m_spCmdMgr->IGetGroupDataFromRegistry(RMX_CMD_GROUP_ID, count, registryIDs, &getDataResult);
	long knownIDs[4] = { RMX_CMD_ID_BASE, RMX_CMD_ID_BASE+1, RMX_CMD_ID_BASE+2, RMX_CMD_ID_BASE+3 };
	if (getDataResult)
	{
		if (!CompareIDs(registryIDs, count, knownIDs, 4))
		{
			ignorePrevious = true;
			LOG_INFO(L"Detect different commands from registry. ignorePrevious=true");
		}
	}

	// create group
	CComPtr<ICommandGroup> spGroup;
	long lErrs;
	CComBSTR bstrTitle, bstrHint;
	bstrTitle.LoadString(IDS_CMD_GROUP_TITLE);
	bstrHint.LoadString(IDS_CMD_GROUP_HINT);
	if (FAILED(m_spCmdMgr->CreateCommandGroup2(RMX_CMD_GROUP_ID, bstrTitle, bstrTitle, bstrHint, -1, ignorePrevious, &lErrs, &spGroup)) || !spGroup)
	{
		LOG_ERROR(L"Failed to create command group");
		return;
	}

	// Set up icon list
	LONG iconIndex = 0;
	SAFEARRAYBOUND dims[1];
	dims[0].lLbound = 0;
	dims[0].cElements = 1;

	CPathEx iconPath(m_rmxRootDir.c_str());
	iconPath /= L"icons\\toolbar_32.bmp";
	SAFEARRAY* sa = SafeArrayCreate(VT_BSTR, 1, dims);
	CComBSTR toolbarIconString(iconPath.c_str());
	SafeArrayPutElement(sa, &iconIndex, toolbarIconString);

	CComVariant vIconList;
	V_VT(&vIconList) = VT_ARRAY | VT_BSTR;
	V_ARRAY(&vIconList) = sa;
	spGroup->put_IconList(vIconList);

	// Set up each command item in the group
	for (auto& cmd : g_rmxCmds)
	{
		CComBSTR tip, hint;
		tip.LoadString(cmd.nTipID);
		hint.LoadString(cmd.nHintID);
		if (FAILED(spGroup->AddCommandItem2(tip, -1, hint, tip, cmd.lImageIndex, cmd.bstrExecuteCallback, cmd.bstrEnableCallback, cmd.lCmdID, swToolbarItem, &cmd.lCmdIndex)))
		{
			LOG_ERROR(L"Failed to create command " << tip);
		}
	}

	// Activate the group
	spGroup->put_HasToolbar(VARIANT_TRUE);
	spGroup->put_HasMenu(VARIANT_FALSE);
	VARIANT_BOOL vActivated;
	spGroup->Activate(&vActivated);
	if (!vActivated)
	{
		LOG_ERROR(L"Failed to activate command group");
		return;
	}

	// Collect all cmd ids and text styles for command population
	std::vector<long> cmdIDs;
	std::vector<long> textStyles;
	for (auto& cmd : g_rmxCmds)
	{
		// Note: Command ID is only avaliable after ICommandGroup->Activate
		spGroup->get_CommandID(cmd.lCmdIndex, &cmd.lCmdID);
		cmdIDs.push_back(cmd.lCmdID);
		textStyles.push_back(swCommandTabButton_TextBelow);
	}

	// Create tab for each document type
	VARIANT_BOOL bRet = VARIANT_FALSE;
	long docTypes[] = { swDocPART, swDocASSEMBLY, swDocDRAWING };
	for (auto docType : docTypes)
	{
		CComPtr<ICommandTab> spTab;
		// check if tab exists
		m_spCmdMgr->GetCommandTab(docType, bstrTitle, &spTab);

		//If tab exists, but we have ignored the registry info, re-create the tab. Otherwise the ids won't matchup and the tab will be blank 
		if ((spTab != NULL) && !getDataResult || ignorePrevious)
		{
			bRet = VARIANT_FALSE;
			m_spCmdMgr->RemoveCommandTab(spTab, &bRet);
			LOG_INFO(L"Old command tab removed");
			spTab = NULL;
		}
		
		//If cmdTab is null, must be first load (possibly after reset), add the commands to the tabs
		if (spTab == NULL)
		{
			if (FAILED(m_spCmdMgr->AddCommandTab(docType, bstrTitle, &spTab)) || !spTab)
			{
				LOG_ERROR(L"Failed to create command tab");
				return;
			}
			CComPtr<ICommandTabBox> spTabBox;
			if (FAILED(spTab->AddCommandTabBox(&spTabBox)) || !spTabBox)
			{
				LOG_ERROR(L"Failed to create command tab box");
				return;
			}

			// populate the commands on this tab box
			bRet = VARIANT_FALSE;
			spTabBox->IAddCommands((long)g_rmxCmds.size(), cmdIDs.data(), textStyles.data(), &bRet);
			if (bRet) {
				LOG_INFO(L"Command buttons populated for doctype " << docType);
			}	
		}
		
	}

	//Clean up
	delete[] registryIDs;
	
	LOG_INFO(L"Command manager initialized");
}

void CSldWorksRMX::RemoveCommandMgr()
{
	if (m_spCmdMgr)
	{
		VARIANT_BOOL bRet;
		m_spCmdMgr->RemoveCommandGroup(RMX_CMD_GROUP_ID, &bRet);
		if (bRet == VARIANT_TRUE)
		{
			LOG_INFO(L"Command manager removed");
		}
		else
		{
			LOG_ERROR(L"Failed to remove command manager");
		}

		m_spCmdMgr.Release();
		g_rmxCmds.clear();
	}
}

BSTR CSldWorksRMX::GetCurrentFile()
{
	CComBSTR ret;
	CComPtr<IModelDoc2> iSwModel;

	iSwApp->get_IActiveDoc2(&iSwModel);
	if (iSwModel != NULL)
	{
		iSwModel->GetPathName(&ret);
		LOG_DEBUG("Currently Active File = " << ret);
	}
	iSwModel.Release();
	return ret.Detach();
}

//Pre - notifies the user program of a FileOpenNotify2 event.
STDMETHODIMP CSldWorksRMX::OnFileOpenPreNotify(BSTR fileName) {
	LOG_DEBUG("FileName = " << fileName);
	
	wstring pathToFile = FileUtils::GetParentPath(fileName);
	//Check if the file is being opened from workspace folder
	if (CSwUtils::GetInstance()->IsWorkSpaceFolder(pathToFile)) {
		bool isProtected = false;
		if (CSwRMXMgr::GetInstance()->RMX_RPMGetFileStatus(fileName, &isProtected) && isProtected) {

			if (CSwRMXMgr::GetInstance()->RMX_IsRPMFolder(pathToFile)) {
				//Append nxl to the fileName
				wstring nxlFileName = FileUtils::bstr2ws(fileName) + NXL_FILE_EXT;
				LOG_DEBUG("Appending NXL to the file name" << nxlFileName.c_str());

				if (!_wrename(fileName, nxlFileName.c_str()))
				{
					//This will happen if the file was opened from Teamcenter and downloaded to workspace folder but file doesn't contains .nxl extension.
					LOG_INFO("Protected File was missing .nxl extension. Appended with '.nxl' extension for file = " << fileName);
				}

				//Workaround: Bug 62456 There will be a mark saying 'Cant determine whether a newer version is in Teamcenter' when Save a protected file back to TC
				wstring regFile = pathToFile + L"\\" + L"swim.txr";
				LOG_INFO("Refreshing swim registry file.");
				SwimUtils::RefreshSwimRegistry(regFile);
			}

			//Check if the file being opened is already set as Read-Only
			bool isReadOnly = FileUtils::GetFileAttributeBit(fileName, FILE_ATTRIBUTE_READONLY);
			if (!isReadOnly) {
				if (CSwRMXMgr::GetInstance()->RMX_GetFileReadOnly(fileName, isReadOnly) && isReadOnly) {
					LOG_INFO("Marking file as read-only. FileName = " << fileName);
					//Mark the file residing in workspace folder as read only
					FileUtils::SetFileAttributeBit(fileName, FILE_ATTRIBUTE_READONLY);
				}
			}
		}
	}
	
	CSwAuthResult authResultObj(fileName);
	if (!CSwUserAuthorization::GetInstance()->OnFileViewAccessCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		//return S_FALSE;
	}

	return S_OK;

}
STDMETHODIMP CSldWorksRMX::OnFileClose(BSTR fileName, long closeReason) {
	LOG_INFO_FMT("FileName:'%s' Reason=%d", fileName, closeReason);
	if (closeReason == swFileCloseNotifyReason_CloseForReload) {
		//drawing file is closed for reprotect
		//TODO: protect file and set name back;
		//iSwApp->SetPromptFilename2(fileName,);
	}
	return S_OK;

}

//Post-notifies the user program when an existing file has been opened. 
//STDMETHODIMP CSldWorksRMX::OnFileOpenNotify(BSTR fileName) {
//	LOG_INFO("FileName = " << fileName);
//	return S_OK;
//
//}

STDMETHODIMP CSldWorksRMX::OnAppCommandCloseNotify(int command, int reason) {
	LOG_DEBUG_FMT("Command = %d, Reason=%d", command, reason);
	if (m_IsWndHookStarted) {
		// Don't stop for JT Export. It wil be unhooked when export dialog gets closed.
		LOG_INFO_FMT("Stopping Window Hook. Command = %d , Reason = %d", command, reason);
		CSwWndHook::Stop();
		m_IsWndHookStarted = false;
	}
	ProcessRecords();

	LOG_DEBUG_FMT(L"m_commandHandlers.size=%lu", m_commandHandlers.size());
	for (auto icmdHandler : m_commandHandlers)
	{
		icmdHandler->OnAppCommandCloseNotify(command, reason);
	}
	return S_OK;
}

void CSldWorksRMX::ProcessRecords() {
	size_t num_records = FileDetails::ProtectFileList.size();
	LOG_DEBUG("Records count = " << num_records);
	if (num_records > 0) {
		//Apply protection to the records contained in the vector
		for (auto fileInstance : FileDetails::ProtectFileList) {
			//File present in the list
			LOG_INFO("File Protection Begin :" << fileInstance.GetFileName());
			bool status = CSwRMXMgr::GetInstance()->ProtectFile(fileInstance.GetFileName(),fileInstance.GetFileTag());
			if (!status) {
				LOG_ERROR("Failed to protect the file = " << fileInstance.GetFileName());
			}
			LOG_INFO("File Protection End :" << fileInstance.GetFileName());
		}
	}
	FileDetails::ProtectFileList.clear();

}

//STDMETHODIMP CSldWorksRMX::OnAppNonNativeFileOpenNotify(BSTR fileName) {
//	LOG_INFO("FileName = " << fileName);
//	return S_OK;
//}

STDMETHODIMP CSldWorksRMX::OnAppBeginTranslationNotify(BSTR fileName, int options) {
	LOG_DEBUG_FMT("FileName = %s, Options = %d", fileName, options);
	CSwAuthResult authResultObj(fileName);
	if (!CSwUserAuthorization::GetInstance()->OnFileViewAccessCheck(authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		return S_FALSE;
	}
	//Check if the file being opened is protected and from non rpm folder.
	//At this moment we don't support opening a nxl file from non rpm folder.
	bool isProtected = false;
	CSwRMXMgr *swRMXMgrInstance = CSwRMXMgr::GetInstance();
	wstring filePath = FileUtils::GetParentPath(fileName);
	if(swRMXMgrInstance->RMX_RPMGetFileStatus(fileName, &isProtected) && isProtected) {
		if (!swRMXMgrInstance->RMX_IsRPMFolder(filePath)) {
			LOG_ERROR(L"Cannot open the protected file. You need to set the folder as secured folder before opening the file. FileName = " << FileUtils::bstr2ws(fileName));
			wstring msg = L"Cannot open the protected file. " + FileUtils::bstr2ws(fileName) +L"\nYou need to set the folder as secured folder before opening the file.";
			CSwResult swResultObj;
			swResultObj.SetErrorMsg(msg);
			swResultObj.SetMessageType(MsgType::MSG_ERROR);
			CSldWorksRMXDialog rmxDialogObj;
			rmxDialogObj.ShowMessageDialog(&swResultObj);
			return S_FALSE;
		}
	}

return S_OK;
}
//
//STDMETHODIMP CSldWorksRMX::OnAppEndTranslationNotify(BSTR fileName, int options) {
//	LOG_INFO_FMT("FileName = %s, Options = %d", fileName, options);
//	return S_OK;
//}


void CSldWorksRMX::AddRPMFolder(BSTR fileName) {
	LOG_INFO("FileName = " << fileName);
	CSwRMXMgr* swRMXMgrInstance = CSwRMXMgr::GetInstance();
	//Check if the CWD is a RPM folder.
	wstring fileDir = FileUtils::GetParentPath(fileName);
	if (!swRMXMgrInstance->RMX_IsRPMFolder(fileDir)) {
		LOG_INFO("Setting directory as RPM. FileDirectory = " << fileDir);
		if (!swRMXMgrInstance->RMX_AddRPMFolder(fileDir)) {
			LOG_WARN("Failed to set directory as RPM = " << fileDir);
		}
		RPMDirList.push_back(fileDir);
	}
}

STDMETHODIMP CSldWorksRMX::OnAppCommandOpenPreNotify(int command, int userCommand) {
	LOG_DEBUG_FMT("Command = %d , UserCommand = %d", command, userCommand);

	CSwWndHook::UnhookJTExport();
	// No control for the RMX customized commands
	if (IsRMXCommand(userCommand))
		return S_OK;

	CSwAuthResult authResultObj;
	if (!CSwUserAuthorization::GetInstance()->PerformCommandPreExecutionCheck(command, userCommand, authResultObj)) {
		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowMessageDialog(&authResultObj);
		LOG_ERROR_FMT("User is not authorized to perform this operation. Command = %d, UserCommand = %d", command, userCommand);
		return S_FALSE;
	}

	if (IsSwCommandUsingWndHook(command, userCommand)) {
		LOG_INFO_FMT("Starting Window Hook. Command = %d , userCommand = %d", command, userCommand);
		CSwWndHook::Start();
		m_IsWndHookStarted = true;
	}
	else if (command == swCommands_User_Toolbar_Min)
	{
		LOG_INFO_FMT("Starting Window Hook for JT Export. Command = %d , userCommand = %d", command, userCommand);
		CSwWndHook::HookJTExport();
	}
	LOG_DEBUG_FMT(L"m_commandHandlers.size=%lu", m_commandHandlers.size());
	for (auto icmdHandler : m_commandHandlers)
	{
		icmdHandler->OnAppCommandOpenPreNotify(command, userCommand);
	}
	return S_OK;
}

STDMETHODIMP CSldWorksRMX::OnAppReferencedFilePreNotify(BSTR fileName, int fileStatus) {
	LOG_DEBUG_FMT("FileName = %s, FileStatus = %d", fileName, fileStatus);
	//Check if file pointed by fileName exists in the folder
	if (!FileUtils::IsFileExists(fileName)) {
		LOG_DEBUG_FMT("File %s doesn't exist", fileName);
		//Check if fileName with nxl extension exists in the folder
		wstring newFileName = wstring(fileName, SysStringLen(fileName)) + NXL_FILE_EXT;
		if (FileUtils::IsFileExists(newFileName)) {
			LOG_DEBUG_FMT("File %s exists", newFileName.c_str());
			//Check if the file being opened is protected and from non rpm folder.
			//At this moment we don't support opening a nxl file from non rpm folder.
			wstring filePath = FileUtils::GetParentPath(fileName);
			if (!CSwRMXMgr::GetInstance()->RMX_IsRPMFolder(filePath)) {
				LOG_ERROR(L"Cannot load the protected reference file. You need to set the referenced folder as secured folder before opening the file. FileName = " << FileUtils::bstr2ws(fileName));
				wstring msg = L"Cannot load the protected reference file. " + FileUtils::bstr2ws(fileName) + L"\nYou need to set the referenced folder as secured folder before opening the file.";
				CSwResult swResultObj;
				swResultObj.SetErrorMsg(msg);
				swResultObj.SetMessageType(MsgType::MSG_ERROR);
				CSldWorksRMXDialog rmxDialogObj;
				rmxDialogObj.ShowMessageDialog(&swResultObj);
				return S_FALSE;
			}

		}

	}
	return S_OK;
}

class ProtectDocHelper {
public:
	ProtectDocHelper(CSldWorksRMX* addinPtr, const wstring& filePath, bool withRefs):m_addin(addinPtr), m_rootDocPath(filePath){
		auto utils = CSwUtils::GetInstance();
		//build docs to be protected
		//TBD:remove docs are already protected?
		m_docsToBeProtected.push_back(filePath);
		auto modelDoc = utils->GetModelDocByFileName(filePath);
		modelDoc->GetType(&m_rootDocType);
		if (withRefs) {
			//Get the modelDoc pointer by fileName.
			auto dependents = GetDependentFiles(modelDoc);
			m_docsToBeProtected.insert(m_docsToBeProtected.end(), dependents.begin(), dependents.end());
		}
		modelDoc.Release();
		LOG_DEBUG_FMT("nPotectingFiles=%d", m_docsToBeProtected.size());
	}
	bool SaveActiveDocs() {
		//TODO:
		//TBD:save dirty docs in addtional closing docs?
		return true;
	}
	const unordered_map<wstring, long>& LoadAdditionalClosingDocs() {
		auto utils = CSwUtils::GetInstance();
		m_additionalDocsToBeClosed.clear();
		if (m_rootDocPath.empty()||m_docsToBeProtected.empty())
			return m_additionalDocsToBeClosed;
		//build docs to be closed
		//get openning windows
		vector<pair<CComPtr<IModelDoc2>, bool>> openDocs;
		utils->GetOpenedModelDocs(openDocs);
		LOG_DEBUG_FMT("nOpenedDocs=%d", openDocs.size());
		for (auto openDoc : openDocs) {
			CComBSTR bstrDocPath;
			openDoc.first->GetPathName(&bstrDocPath);
			wstring docPath = FileUtils::bstr2ws(bstrDocPath);
			CComPtr<IModelDocExtension> swModelDocExt;
			openDoc.first->get_Extension(&swModelDocExt);
			long nExtensionModelView;
			swModelDocExt->GetModelViewCount(&nExtensionModelView);
			LOG_DEBUG_FMT("'%s':nExtensionModelViews=%d", docPath.c_str(), nExtensionModelView);
			if (nExtensionModelView > 0
				&& _wcsicmp(docPath.c_str(), m_rootDocPath.c_str()) != 0)
			{
				long docType = 0;
				openDoc.first->GetType(&docType);
				//this document is a root file of view
				//check if it includes any docs to be closed
				if (std::find(m_docsToBeProtected.begin(), m_docsToBeProtected.end(), docPath) != m_docsToBeProtected.end()) {
					LOG_DEBUG_FMT("'%s':need to be closed", docPath.c_str());
					m_additionalDocsToBeClosed.insert(std::make_pair(docPath, docType));
					continue;
				}
				//find if dependents of root file need to be closed
				auto viewDependents = GetDependentFiles(openDoc.first);
				LOG_DEBUG_FMT("'%s':nDepends=%d", docPath.c_str(), viewDependents.size());
				for (auto viewDependent : viewDependents) {
					if (std::find(m_docsToBeProtected.begin(), m_docsToBeProtected.end(), viewDependent) != m_docsToBeProtected.end()) {
						LOG_DEBUG_FMT("'%s':dependent file('%s') need to be closed", docPath.c_str(), viewDependent.c_str());
						m_additionalDocsToBeClosed.insert(std::make_pair(docPath, docType));
						break;
					}
					LOG_DEBUG_FMT("'%s':dependent file('%s')", docPath.c_str(), viewDependent.c_str());
				}
			}
		}
		LOG_DEBUG_FMT("nReferencingDocs=%d", m_additionalDocsToBeClosed.size());
		return m_additionalDocsToBeClosed;
	}
	bool CloseDocs() {
		if (m_rootDocPath.empty())
			return false;
		LoadAdditionalClosingDocs();
		for (auto kvp : m_additionalDocsToBeClosed) {
			if (!CloseDoc(kvp.first))
			{
				return false;
			}
		}
		//close active doc after additional docs
		CloseDoc(m_rootDocPath);
		theSwUtils->ListOpenedModelDocs();
		return true;
	}
	void ReopenDocs() {
		if (m_rootDocPath.empty())
			return;
		for (auto kvp : m_additionalDocsToBeClosed) {
			OpenDoc(kvp.first, kvp.second);
		}
		//reopen active doc after additional docs to make it active
		OpenDoc(m_rootDocPath, m_rootDocType);
		theSwUtils->ListOpenedModelDocs();
	}
	const vector<wstring>& GetDocsToBeProtected() {
		return m_docsToBeProtected;
	}
private:
	vector<wstring> GetDependentFiles(CComPtr<IModelDoc2> modelDoc) {
		vector<wstring> dependents;
		VARIANT_BOOL traverseFlag = VARIANT_TRUE;
		VARIANT_BOOL searchFlag = VARIANT_FALSE;
		VARIANT_BOOL addReadOnlyInfo = VARIANT_FALSE;
		long referencedDocCount = 0;
		modelDoc->IGetNumDependencies2(traverseFlag, searchFlag, addReadOnlyInfo, &referencedDocCount);

		if (referencedDocCount > 0)
		{
			BSTR* dependencyDocuments = new BSTR[referencedDocCount];
			/*
			If you use IGetDependencies2() method with an assembly that contains two documents, Part1 and SubAssem1,
			An example of what might be returned is :
			["Part1", "C:\temp\Part1.SLDPRT", "SubAssem1", "c:\temp\SubAssem1.SLDASM"]
			*/
			modelDoc->IGetDependencies2(traverseFlag, searchFlag, addReadOnlyInfo, dependencyDocuments);
			for (int ind = 0; ind < referencedDocCount; ind += 2) {
				auto modeldoc = theSwUtils->GetModelDocByFileName(dependencyDocuments[ind + 1]);
				if (modeldoc == nullptr || theSwUtils->IsVirtualModel(modeldoc)) {
					continue;
				}
				dependents.push_back(dependencyDocuments[ind + 1]);
			}
			for (int i = 0; i < referencedDocCount; i++)
				SysFreeString(dependencyDocuments[i]);
			delete[] dependencyDocuments;
		}

		return dependents;
	}
	bool CloseDoc(const wstring& docPath) {
		LOG_DEBUG_FMT("Closing-'%s'", docPath.c_str());
		CComBSTR bstrName(FileUtils::GetFileNameWithExt(docPath).c_str());
		if (FAILED(m_addin->GetSldWorksPtr()->QuitDoc(bstrName)))
		{
			LOG_ERROR_FMT(L"Failed to close the document-'%s'", docPath.c_str());
			return false;
		}
		return true;
	}
	bool OpenDoc(const wstring &docPath, long docType) {
		LOG_DEBUG_FMT("Resuming-'%s'", docPath.c_str());
		long lWarns, lErrors;
		CComPtr<IModelDoc2> newdoc;
		CComBSTR bstrFilePath(docPath.size(), docPath.data());
		if (FAILED(m_addin->GetSldWorksPtr()->OpenDoc6(bstrFilePath, docType, swOpenDocOptions_Silent, L"", &lErrors, &lWarns, &newdoc))
			|| !newdoc)
		{
			LOG_ERROR_FMT(L"Failed to reopen-'%s'", docPath.c_str());
			return false;
		}
		return true;
	}
	CSldWorksRMX* m_addin;
	wstring m_rootDocPath;
	long m_rootDocType;
	vector<wstring> m_docsToBeProtected;
	unordered_map<wstring,long> m_additionalDocsToBeClosed;//the root docs which has reference to the docs to be protected
};

//
//  Callback when the RMX command is fired
//
STDMETHODIMP CSldWorksRMX::OnRMXProtect(void)
{
	LOG_INFO(L"Starting protect...");
	CComPtr<IModelDoc2> spDoc;
	if (FAILED(iSwApp->get_IActiveDoc2(&spDoc)) || !spDoc)
		return S_FALSE;

	VARIANT_BOOL vRet = VARIANT_FALSE;
	CComBSTR bstrTitle;
	spDoc->GetTitle(&bstrTitle);
	wstring wsTitle = FileUtils::bstr2ws(bstrTitle);

	CComBSTR bstrFileName;
	spDoc->GetPathName(&bstrFileName);
	VARIANT_BOOL bDirty = VARIANT_FALSE;
	spDoc->GetSaveFlag(&bDirty);
	wstring wsFileName = FileUtils::bstr2ws(bstrFileName);
	if (!FileUtils::IsFile(wsFileName))
	{
		LOG_INFO(L"Saving new document" << wsTitle.c_str());
		CComPtr<IModelDocExtension> spDocEx;
		spDoc->get_Extension(&spDocEx);
		if (!spDocEx)
		{
			LOG_ERROR(L"Failed to save new document. Invalid IModelDocExtension pointer");
			return S_FALSE;
		}

		spDocEx->RunCommand(swCommands_SaveAs, L"", &vRet);
		if (!vRet)
		{
			LOG_ERROR(L"Failed to save new document: " << wsTitle.c_str());
			return S_FALSE;
		}
	}
	else if (bDirty)
	{
		// Save if it's new doc or it's modified in memory
		LOG_INFO(L"Saving modified document" << wsTitle.c_str());
		CComBSTR bstrMsg;
		bstrMsg.LoadString(IDS_WARN_SAVE_CHANGE_BEFORE_PROTECT);
		long lRet = theSwUtils->ShowMessageDialog(FileUtils::bstr2ws(bstrMsg), swMbQuestion, swMbYesNoCancel);
		if (lRet == swMbHitYes)
		{
			long lWarns, lErrors;
			if (FAILED(spDoc->Save3(swSaveAsOptions_Silent, &lErrors, &lWarns, &vRet)))
			{
				LOG_ERROR_FMT(L"Failed to save modified document.(error: %l, warning: %l)", lErrors, lWarns);
				return S_FALSE;
			}
		}
		else if(lRet == swMbHitCancel)
		{
			LOG_ERROR(L"Protect cancelled by user : " << wsTitle.c_str());
			return S_FALSE;
		}
		else
		{
			LOG_WARN(L"Ignore to save modification. " << wsTitle.c_str());
		}
	}
	// Referesh file object
	spDoc.Release();
	if (FAILED(iSwApp->get_IActiveDoc2(&spDoc)) || !spDoc)
		return S_FALSE;

	bstrFileName.Empty();
	spDoc->GetPathName(&bstrFileName);
	wsFileName = FileUtils::bstr2ws(bstrFileName);

	if (!FileUtils::IsFile(wsFileName))
	{
		LOG_ERROR(L"Failed to protect. File still not found.");
		return S_FALSE;
	}

	wstring wsTags;
	CComBSTR bstrButtonLabel;
	bstrButtonLabel.LoadString(IDS_CMD_PROTECT_TIP);
	if (theSwRMXMgr->RMX_ShowProtectDialog(wsFileName, FileUtils::bstr2ws(bstrButtonLabel), wsTags))
	{
		ProtectDocHelper protectDocHelper(this, wsFileName, false);
		//TODO:handling close failure
		if (!protectDocHelper.CloseDocs())
		{
			return S_FALSE;

		}
		LOG_INFO(L"Protecting document" << wsTitle.c_str());
		if (theSwRMXMgr->ProtectFile(wsFileName, wsTags, true))
		{
			protectDocHelper.ReopenDocs();
		}
	}

	return S_OK;
}

//
// Callback when the RMX command is going to populate, to determine if button is disable
//
STDMETHODIMP CSldWorksRMX::OnRMXProtectEnable(long* status)
{
	*status = 0;
	
	// Disable in manged mode
	if (!IsSwimAddinActive())
	{
		CComPtr<IModelDoc2> spCurDoc;
		if (SUCCEEDED(iSwApp->get_IActiveDoc2(&spCurDoc))
			&& spCurDoc)
		{
			//disable for virtual model
			if (!theSwUtils->IsVirtualModel(spCurDoc)) {
				// Disable if file already protected
				const auto& curFileName = theSwUtils->GetCurrentActiveFile();
				if (FileUtils::IsFile(curFileName))
				{
					bool isProtected = false;
					if (!theSwRMXMgr->RMX_RPMGetFileStatus(curFileName, &isProtected) 
						|| !isProtected) {
						*status = 1;
					}
				}
			}
		}
	}

	return S_OK;
}

STDMETHODIMP CSldWorksRMX::OnRMXViewInfo(void)
{
	LOG_INFO(L"");

	theSwRMXMgr->RMX_ShowFileInfoDialog(theSwUtils->GetCurrentActiveFile());

	return S_OK;
}

STDMETHODIMP CSldWorksRMX::OnRMXViewInfoEnable(long* status)
{
	*status = 0;

	CComPtr<IModelDoc2> spCurDoc;
	if (SUCCEEDED(iSwApp->get_IActiveDoc2(&spCurDoc)) 
		&& spCurDoc)
	{
		//disable for virtual model
		if (!theSwUtils->IsVirtualModel(spCurDoc)) {
			const auto& curFileName = theSwUtils->GetModelNameForModelDoc(spCurDoc);
			if (FileUtils::IsFile(curFileName))
			{
				bool isProtected = false;
				if (theSwRMXMgr->RMX_RPMGetFileStatus(curFileName, &isProtected) && isProtected) {
					*status = 1;
				}
			}
		}
	}

	return S_OK;
}

STDMETHODIMP CSldWorksRMX::OnRMXProtectWithRefs(void)
{
	LOG_INFO(L"Starting protect...");
	CComPtr<IModelDoc2> spRootDoc;
	if (FAILED(iSwApp->get_IActiveDoc2(&spRootDoc)) || !spRootDoc)
		return S_FALSE;

	VARIANT_BOOL vRet = VARIANT_FALSE;
	CComBSTR bstrTitle;
	spRootDoc->GetTitle(&bstrTitle);
	wstring wsRootTitle = FileUtils::bstr2ws(bstrTitle);

	CComBSTR bstrRootFileName;
	spRootDoc->GetPathName(&bstrRootFileName);
	VARIANT_BOOL bDirty = VARIANT_FALSE;
	spRootDoc->GetSaveFlag(&bDirty);
	wstring wsRootFileName = FileUtils::bstr2ws(bstrRootFileName);
	if (!FileUtils::IsFile(wsRootFileName))
	{
		LOG_INFO(L"Saving new document" << wsRootTitle.c_str());
		CComPtr<IModelDocExtension> spDocEx;
		spRootDoc->get_Extension(&spDocEx);
		if (!spDocEx)
		{
			LOG_ERROR(L"Failed to save new document. Invalid IModelDocExtension pointer");
			return S_FALSE;
		}

		spDocEx->RunCommand(swCommands_SaveAs, L"", &vRet);
		if (!vRet)
		{
			LOG_ERROR(L"Failed to save new document: " << wsRootTitle.c_str());
			return S_FALSE;
		}
	}
	else if (bDirty)
	{
		// Save if it's new doc or it's modified in memory
		CComBSTR bstrMsg;
		bstrMsg.LoadString(IDS_WARN_SAVE_CHANGE_BEFORE_PROTECT);
		long lRet = theSwUtils->ShowMessageDialog(FileUtils::bstr2ws(bstrMsg), swMbQuestion, swMbYesNoCancel);
		if (lRet == swMbHitYes)
		{
			long lWarns, lErrors;
			if (FAILED(spRootDoc->Save3(swSaveAsOptions_Silent | swSaveAsOptions_SaveReferenced, &lErrors, &lWarns, &vRet)))
			{
				LOG_ERROR_FMT(L"Failed to save modified document.(error: %l, warning: %l)", lErrors, lWarns);
				return S_FALSE;
			}
		}
		else if (lRet == swMbHitCancel)
		{
			LOG_ERROR(L"Protect cancelled by user : " << wsRootTitle.c_str());
			return S_FALSE;
		}
		else
		{
			LOG_WARN(L"Ignore to save modification. " << wsRootTitle.c_str());
		}
	}
	// Referesh file object
	spRootDoc.Release();
	if (FAILED(iSwApp->get_IActiveDoc2(&spRootDoc)) || !spRootDoc)
		return S_FALSE;

	bstrRootFileName.Empty();
	spRootDoc->GetPathName(&bstrRootFileName);
	wsRootFileName = FileUtils::bstr2ws(bstrRootFileName);

	if (!FileUtils::IsFile(wsRootFileName))
	{
		LOG_ERROR(L"Failed to protect. File still not found.");
		return S_FALSE;
	}
	wstring wsTags;
	CComBSTR bstrButtonLabel;
	bstrButtonLabel.LoadString(IDS_CMD_PROTECT_TIP);
	if (theSwRMXMgr->RMX_ShowProtectDialog(wsRootFileName, FileUtils::bstr2ws(bstrButtonLabel), wsTags))
	{
		ULONGLONG tStart = GetTickCount64();
		ProtectDocHelper protectDocHelper(this, wsRootFileName, true);
		wstring successReport, failedReport;

		protectDocHelper.CloseDocs();
		
		// Scan all referenced docs to see if any of them is not protected yet.
		for (auto docToBeProtected : protectDocHelper.GetDocsToBeProtected())
		{
			bool isProtected = false;
			if (!CSwRMXMgr::GetInstance()->RMX_RPMGetFileStatus(docToBeProtected, &isProtected) || !isProtected) {
				LOG_INFO_FMT(L"'%s':Protecting...", docToBeProtected.c_str());
				if (theSwRMXMgr->ProtectFile(docToBeProtected, wsTags, true))
				{
					successReport += docToBeProtected;
					successReport += L"\n";
				}
				else
				{
					failedReport += docToBeProtected;
					failedReport += L"\n";
				}
			}
			else if (isProtected)
			{
				LOG_INFO_FMT(L"'%s':is already protected", docToBeProtected.c_str());
			}
		}
		// Reopen
		protectDocHelper.ReopenDocs();

		ULONGLONG tEnd = GetTickCount64();

		LOG_DEBUG(L"Protect with dependencies report:");
		LOG_DEBUG(L"\t- Tags: " << wsTags.c_str());
		LOG_DEBUG(L"\t- Succeed: " << successReport.c_str());
		LOG_DEBUG(L"\t- Failure: " << failedReport.c_str());
		LOG_DEBUG_FMT(L"\t- Time elapsed: %llu ms", tEnd - tStart);

	}

	return S_OK;
}

STDMETHODIMP CSldWorksRMX::OnRMXProtectWithRefsEnable(long* status)
{
	*status = 0;
	// Disable in managed mode
	if (!IsSwimAddinActive())
	{
		CComPtr<IModelDoc2> spCurDoc;
		if (FAILED(iSwApp->get_IActiveDoc2(&spCurDoc)) || !spCurDoc)
		{
			LOG_ERROR(L"Failed to protect. Current document not found");
			return S_FALSE;
		}
		//disable if root file is virtual
		if (!theSwUtils->IsVirtualModel(spCurDoc)) {

			// This command is only avaliable for drawing and assembly.
			long lDocType;
			spCurDoc->GetType(&lDocType);
			if ((lDocType != swDocASSEMBLY) && (lDocType != swDocDRAWING))
				return S_OK;

			CComBSTR bstrFileName;
			spCurDoc->GetPathName(&bstrFileName);
			wstring wsCurFileName = FileUtils::bstr2ws(bstrFileName);
			if (!FileUtils::IsFile(wsCurFileName))
			{
				// Enable if file is never saved to disk yet.
				*status = 1;
			}
			else
			{
				bool isProtected = false;
				if (!theSwRMXMgr->RMX_RPMGetFileStatus(wsCurFileName, &isProtected) || !isProtected) {
					// Enable if file not protected
					*status = 1;
				}
				else
				{
					// Scan all referenced docs to see if any of them is not protected yet.
					const vector<wstring>& referencedComponents = CSwUtils::GetInstance()->GetReferencedFilesOfModelDoc(spCurDoc);
					for (const auto& refComp : referencedComponents) {
						isProtected = false;
						auto modeldoc = theSwUtils->GetModelDocByFileName(refComp);
						if (modeldoc == nullptr || theSwUtils->IsVirtualModel(modeldoc)) {
							continue;
						}
						if (!CSwRMXMgr::GetInstance()->RMX_RPMGetFileStatus(refComp, &isProtected) || !isProtected) {
							*status = 1;
							break;
						}
					}
				}
			}
		}
	}
		
	return S_OK;
}

STDMETHODIMP CSldWorksRMX::OnRMXAbout(void)
{
	LOG_INFO(L"");
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		CSldWorksRMXDialog rmxDialogObj;
		rmxDialogObj.ShowAboutDialog();
	}
	return S_OK;
}

bool CSldWorksRMX::IsRMXCommand(long cmdID)
{
	auto elem = std::find_if(g_rmxCmds.cbegin(), g_rmxCmds.cend(), [&cmdID](const sRMXCMD& cmd)
	{
		return cmdID == cmd.lCmdID;
	});

	return (elem != g_rmxCmds.end());
}

bool CSldWorksRMX::IsSwimAddinActive()
{
	CComPtr<IDispatch> spSwAddin;
	iSwApp->GetAddInObject(L"TranscenData.SwimAddin.SwAddin", &spSwAddin);
	return (spSwAddin != NULL);
}

void CSldWorksRMX::InitRPMFolders()
{
	LOG_INFO(L"Seting up RPM folders...");
	static map<swUserPreferenceStringValue_e, wstring> swLocationTypes = {
		{swFileLocationsDocumentTemplates, L"swFileLocationsDocumentTemplates"},
		{swFileLocationsDocuments, L"swFileLocationsDocuments"},
		{swAutoSaveDirectory, L"swAutoSaveDirectory"},
		{swBackupDirectory, L"swBackupDirectory"}
	};

	for (auto locType : swLocationTypes)
	{
		wstring dir;
		theSwUtils->GetSwUserPreferenceStringValue(locType.first, dir);
		LOG_INFO_FMT(L"Read user preference for %s: %s", locType.second.c_str(), dir.c_str());
		if (!dir.empty()) {
			vector<wstring> vecDirs;
			FileUtils::wsTokenizer(dir, L";", vecDirs);
			for (const auto& d : vecDirs) {
				if (!theSwRMXMgr->RMX_IsRPMFolder(d)) {
					theSwRMXMgr->RMX_AddRPMFolder(d);

					// RPM folder needs to be reset when unloading RMX
					if (locType.first == swFileLocationsDocumentTemplates)
						RPMDirList.push_back(d);
				}
			}
		}
	}

	//Set the system tmp folder as RPM. This location is used by SolidWorks or Java.exe to create preview files in managed mode.
	/*DWORD dwRetVal = 0;
	TCHAR lpTempPathBuffer[MAX_PATH];
	dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
	if (dwRetVal > MAX_PATH || (dwRetVal == 0))
	{
		LOG_ERROR(TEXT("GetTempPath failed"));
	}
	else {
		std::wstring tmpFilePath = wstring(lpTempPathBuffer);
		if (!swRMXMgrInstance->RMX_IsRPMFolder(tmpFilePath)) {
			swRMXMgrInstance->RMX_AddRPMFolder(tmpFilePath);
			RPMDirList.push_back(tmpFilePath);
		}
	}*/
}
