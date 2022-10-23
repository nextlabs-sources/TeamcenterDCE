#include "SwUtils.h"
#include "FileUtils.h"
#include "SwRMXMgr.h"
#include "SwFileTypes.h"
#include "SwimUtils.h"

CSwUtils* CSwUtils::m_SwUtilsInstance = NULL;
CSldWorksRMX * CSwUtils::userAddin = NULL;

CSwUtils::CSwUtils() {
	//CSwUserAuthorization Constructor
	return;
}

CSwUtils::~CSwUtils() {
	//CSwUserAuthorization Destructor
}


CSwUtils * CSwUtils::GetInstance() {
	if (m_SwUtilsInstance != nullptr) {
		return m_SwUtilsInstance;
	}
	else {
		m_SwUtilsInstance = new CSwUtils();
		return m_SwUtilsInstance;
	}
}

vector<wstring> CSwUtils::GetReferencedFilesOfModelDoc(CComPtr<IModelDoc2> &modelDoc)
{
	vector<wstring> referencedFileNames;
	VARIANT_BOOL traverseFlag = VARIANT_TRUE;
	VARIANT_BOOL searchFlag = VARIANT_FALSE;
	VARIANT_BOOL addReadOnlyInfo = VARIANT_FALSE;
	long referencedDocCount = 0;
	if (modelDoc == NULL)
		return referencedFileNames;

	modelDoc->IGetNumDependencies2(traverseFlag, searchFlag, addReadOnlyInfo, &referencedDocCount);

	if (referencedDocCount > 0) {
		BSTR* dependencyDocuments = new BSTR[referencedDocCount];
		/*
		If you use IGetDependencies2() method with an assembly that contains two documents, Part1 and SubAssem1,
		An example of what might be returned is :
		["Part1", "C:\temp\Part1.SLDPRT", "SubAssem1", "c:\temp\SubAssem1.SLDASM"]
		*/
		modelDoc->IGetDependencies2(traverseFlag, searchFlag, addReadOnlyInfo, dependencyDocuments);
		for (int ind = 0; ind < referencedDocCount;) {
			referencedFileNames.push_back(dependencyDocuments[ind + 1]);
			ind = ind + 2;
		}
		for (int i = 0; i < referencedDocCount; i++)
			SysFreeString(dependencyDocuments[i]);
		delete[] dependencyDocuments;
	}
	return referencedFileNames;
}

vector<wstring> CSwUtils::GetReferencedModelDocuments(wstring fileName) {
	CComPtr<IModelDoc2> modelDoc = NULL;
	if (!fileName.empty()) {
		LOG_DEBUG("FileName = " << fileName.c_str());
		//Get the modelDoc pointer by fileName.
		modelDoc = GetModelDocByFileName(fileName);
		if (modelDoc == NULL)
		{
			LOG_DEBUG_FMT("ModelDoc for file-'%s' is not found", fileName.c_str());
		}
	}
	if (modelDoc == NULL) {
		userAddin->GetSldWorksPtr()->get_IActiveDoc2(&modelDoc);
		CComBSTR szPathName;
		modelDoc->GetPathName(&szPathName);
		LOG_DEBUG_FMT("Active ModelDoc='%s'", FileUtils::bstr2ws(szPathName).c_str());
	}
	return GetReferencedFilesOfModelDoc(modelDoc);
}


long CSwUtils::ShowMessageDialog(const wstring& message,long icons, long buttons) {
	long lRet = 0;
	CComBSTR bstrMsg((int)message.size(), message.data());
	userAddin->GetSldWorksPtr()->SendMsgToUser2(bstrMsg, icons, buttons, &lRet);
	return lRet;
}


long CSwUtils::GetModelDocType() {
	CComPtr<IModelDoc2> modelDoc = NULL;
	userAddin->GetSldWorksPtr()->get_IActiveDoc2(&modelDoc);
	long docType= swDocNONE;
	if (modelDoc != NULL) {
		modelDoc->GetType(&docType);
	}
	modelDoc.Release();
	return docType;
}

long CSwUtils::GetModelDocTypeByName(const wstring& fileName) {
	LOG_DEBUG("FileName = " << fileName.c_str());
	wstring wfileExtn = FileUtils::GetFileExt(fileName);
	const wchar_t *fileExtn = wfileExtn.c_str();
	if (!_wcsicmp(fileExtn, L".SLDPRT") || !_wcsicmp(fileExtn, L".PRT"))
		return swDocPART;
	else if (!_wcsicmp(fileExtn, L".SLDASM") || !_wcsicmp(fileExtn, L".ASM"))
		return swDocASSEMBLY;
	else if (!_wcsicmp(fileExtn, L".SLDDRW") || !_wcsicmp(fileExtn, L".DRW"))
		return swDocDRAWING;
	else
		return swDocNONE;
}

wstring CSwUtils::GetModelDocTypeByEnum(long type) {
	if (type == swDocPART)
		return L".SLDPRT";
	else if (type == swDocASSEMBLY)
		return L".SLDASM";
	else if (type == swDocDRAWING)
		return L".SLDDRW";
	else
		return L"";
}


wstring CSwUtils::GetCurrentActiveFile()
{
	CComBSTR activeFile;
	CComPtr<IModelDoc2> modelDoc = NULL;
	userAddin->GetSldWorksPtr()->get_IActiveDoc2(&modelDoc);

	if (modelDoc != NULL)
	{
		modelDoc->GetPathName(&activeFile);
		
	}
	modelDoc.Release();

	return FileUtils::bstr2ws(activeFile);
}

CComPtr<IModelDoc2> CSwUtils::GetCurrentlyActiveModelDoc() {
	CComPtr<IModelDoc2> modelDoc = NULL;
	userAddin->GetSldWorksPtr()->get_IActiveDoc2(&modelDoc);
	return modelDoc;
}

bool CSwUtils::IsFileVisible(BSTR fileName) {
	if (fileName == NULL)
		return false;
	LOG_INFO("FileName = " << fileName);
	vector<pair<CComPtr<IModelDoc2>, bool>> openedFileDocs;
	GetOpenedModelDocs(openedFileDocs);
	for (auto openedDoc : openedFileDocs) {
		CComBSTR szPathName;
		openedDoc.first->GetPathName(&szPathName);
		if (!_wcsicmp(fileName, szPathName)) {
			return openedDoc.second;
		}
	}
	return false;
}

int CSwUtils::GetVisibleDocumentsCount() {
	int count = 0;
	vector<pair<CComPtr<IModelDoc2>, bool>> openedFileDocs;
	GetOpenedModelDocs(openedFileDocs);
	for (auto openedDoc : openedFileDocs) {
		if (openedDoc.second) {
			++count;
		}
	}
	return count;

}


bool CSwUtils::IsLastVisibleDocument(const wstring& fileName) {
	int nvisibles = 0;
	bool isThisFileVisible = false;
	bool found = false;
	vector<pair<CComPtr<IModelDoc2>, bool>> openedFileDocs;
	LOG_DEBUG_FMT("FileName='%s'", fileName.c_str());
	if (fileName.empty())
		return false;
	GetOpenedModelDocs(openedFileDocs);
	for (auto openedDoc : openedFileDocs) {
		if (openedDoc.second)
		{
			nvisibles++;
		}
		if (!found)
		{
			CComBSTR szPathName;
			openedDoc.first->GetPathName(&szPathName);
			if (szPathName!=NULL
				&& _wcsicmp(fileName.c_str(), szPathName) == 0) {
				isThisFileVisible = openedDoc.second;
				found = true;
			}
		}
	}
	LOG_DEBUG_FMT("FileName='%s' found=%d visible=%d nVisibles=%d nopened=%d"
		, fileName.c_str(), found, isThisFileVisible, nvisibles, openedFileDocs.size());
	return isThisFileVisible ? nvisibles == 1 : nvisibles == 0;
}

/*
 *	[in]fileName : C:\Users\tcadmin\Siemens\swcache\Default Workspace\Sample.SLDPRT
 *	[out]CComPtr<IModelDoc2>
 */
CComPtr<IModelDoc2> CSwUtils::IsFileOpened(const wstring &filePath) {
	LOG_DEBUG("filePath = " << filePath.c_str());
	vector<pair<CComPtr<IModelDoc2>, bool>> openedFileDocs;
	GetOpenedModelDocs(openedFileDocs);
	for (auto openedDoc : openedFileDocs) {
		CComBSTR szPathName;
		openedDoc.first->GetPathName(&szPathName);
		if (!_wcsicmp(filePath.c_str(), FileUtils::bstr2ws(szPathName).c_str())) {
			LOG_INFO_FMT("File %s found opened in the current solidworks session ", filePath.c_str());
			
			return openedDoc.first;
		}
	}
	return NULL;
}


/*
*	[in]fileName : Sample.SLDPRT
*	[out]modelDoc : CComPtr<IModelDoc2>
*/

CComPtr<IModelDoc2> CSwUtils::IsFileOpened2(const wstring &fileName) {
	LOG_DEBUG("FileName = " << fileName.c_str());
	vector<pair<CComPtr<IModelDoc2>, bool>> openedFileDocs;
	GetOpenedModelDocs(openedFileDocs);
	for (auto openedDoc : openedFileDocs) {
		CComBSTR szPathName;
		openedDoc.first->GetPathName(&szPathName);
		if (!_wcsicmp(fileName.c_str(), FileUtils::GetFileNameWithExt(FileUtils::bstr2ws(szPathName)).c_str())) {
			LOG_INFO_FMT("File %s found opened in the current solidworks session ", fileName.c_str());
			return openedDoc.first;
		}
	}
	return NULL;
}
bool CSwUtils::TryFindRootFileOfVirtualModel(CComPtr<IModelDoc2>& modelDoc, wstring& rootFile)
{
	if (modelDoc == NULL)
		return false;
	CComBSTR szPathName;
	modelDoc->GetPathName(&szPathName);
	rootFile = FileUtils::bstr2ws(szPathName);
	CComPtr<IModelDocExtension> swModelDocExt;
	modelDoc->get_Extension(&swModelDocExt);
	VARIANT pathChain;
	VARIANT titleChain;
	VARIANT_BOOL isVirtual;
	::VariantInit(&pathChain);
	::VariantInit(&titleChain);
	swModelDocExt->IsVirtualComponent3(&pathChain, &titleChain, &isVirtual);

	LOG_DEBUG_FMT("[%s]isVirutal=%d'"
		, rootFile.c_str(), isVirtual);
	if (isVirtual)
	{
		CComPtr<IDispatch> pDispatchSafeArray = NULL;
		CComPtr<ISafeArrayUtility> pSwSafeArray = NULL;
		HRESULT hres;
		hres = userAddin->GetSldWorksPtr()->GetSafeArrayUtility(&pDispatchSafeArray);
		hres = pDispatchSafeArray.QueryInterface<ISafeArrayUtility>(&pSwSafeArray);

		//Get the name of each configuration
		for (int i = 0;; i++) {
			CComBSTR path, title;
			pSwSafeArray->GetBstr(pathChain, i, &path);
			pSwSafeArray->GetBstr(titleChain, i, &title);
			if (path == NULL)
				break;
			//last path is the root file
			rootFile = FileUtils::bstr2ws(path);
			LOG_DEBUG_FMT("[%d]path='%s' title='%s'"
				, i, rootFile.c_str(), FileUtils::bstr2ws(title).c_str());
		}
		return true;
	}
	return false;
}
bool CSwUtils::IsVirtualModel(CComPtr<IModelDoc2> &modelDoc)
{
	if (modelDoc == NULL)
		return false;
	CComPtr<IModelDocExtension> swModelDocExt;
	modelDoc->get_Extension(&swModelDocExt);
	VARIANT pathChain;
	VARIANT titleChain;
	VARIANT_BOOL isVirtual;
	::VariantInit(&pathChain);
	::VariantInit(&titleChain);
	swModelDocExt->IsVirtualComponent3(&pathChain, &titleChain, &isVirtual);
	return isVirtual==VARIANT_TRUE;
}
void CSwUtils::ListOpenedModelDocs() {
	vector<pair<CComPtr<IModelDoc2>, bool>> openedFileDocs;
	GetOpenedModelDocs(openedFileDocs);
	LOG_DEBUG_FMT("nOpenedDocs=%d", openedFileDocs.size());
	for (auto openedDoc : openedFileDocs) {
		CComBSTR szPathName;
		openedDoc.first->GetPathName(&szPathName);
		CComPtr<IModelDocExtension> swModelDocExt;
		openedDoc.first->get_Extension(&swModelDocExt);
		long nExtensionModelView;
		swModelDocExt->GetModelViewCount(&nExtensionModelView);
		//openedDoc.first->GetModelViewCount(&nModelView)//obsoleted
		LOG_DEBUG_FMT("'%s':nExtensionModelViews=%d", FileUtils::bstr2ws(szPathName).c_str(), nExtensionModelView);
	}
}

void CSwUtils::GetOpenedModelDocs(vector<pair<CComPtr<IModelDoc2>,bool>> & openedFileList) {
	LOG_DEBUG("CSwUtils::GetOpenedModelDocs called");
	HRESULT hr = S_OK;
	CComPtr <ISldWorks> pSldWorks;
	CComPtr<IEnumDocuments2> pEnumDoc;
	long nFetched = -1;
	CComPtr<IModelDoc2> pModelDoc;
	pSldWorks = userAddin->GetSldWorksPtr();
	ASSERT(pSldWorks);
	hr = pSldWorks->EnumDocuments2(&pEnumDoc);
	ASSERT(pEnumDoc);
	if (pEnumDoc) {
		hr = pEnumDoc->Reset();
		do
		{
			pModelDoc = NULL;
			hr = pEnumDoc->Next(1, &pModelDoc, &nFetched);
			if (pModelDoc)
			{
				VARIANT_BOOL isVisible;
				pModelDoc->get_Visible(&isVisible);
				bool visibility = (isVisible == VARIANT_FALSE) ? false : true;
				openedFileList.push_back(make_pair(pModelDoc, visibility));
			}

		} while (pModelDoc);
	}

}

long long  CSwUtils::GetModelWindowHandle(BSTR fileName) {
	LOG_INFO("FileName = " << fileName);
	wstring fileExt = FileUtils::GetFileExt(fileName);
	CComPtr<IFrame> iFrameObj;
	userAddin->GetSldWorksPtr()->IFrameObject(&iFrameObj);

	long modelWindowCount;
	iFrameObj->GetModelWindowCount(&modelWindowCount);

	CComPtr<IModelWindow> * iModelWindow = new CComPtr<IModelWindow>[modelWindowCount];

	iFrameObj->IGetModelWindows(modelWindowCount, (IModelWindow**)iModelWindow);
	long long HWnD = 0;

	for (int ind = 0; ind < modelWindowCount; ++ind) {
		CComBSTR modelWindowTitle;
		iModelWindow[ind]->get_Title(&modelWindowTitle);
		//In case of SLDDRW files, the window title is of the form SampleDrawing-Sheet1,SampleDrawing-Sheet2
		if (!_wcsicmp(fileExt.c_str(), L".SLDDRW")) {
			size_t pos;
			std::wstring ws(modelWindowTitle, SysStringLen(modelWindowTitle));
			pos = ws.find_last_of(L"-");
			ws = ws.substr(0, pos);
			ws.erase(std::find_if(ws.rbegin(), ws.rend(), [](wint_t ch) { return !iswspace(ch); }).base(), ws.end());
			modelWindowTitle = CComBSTR((int)ws.size(), ws.data());
		}
		if (FileUtils::GetFileName(modelWindowTitle.m_str) == FileUtils::GetFileName(fileName)) {
			iModelWindow[ind]->get_HWndx64(&HWnD);
		}
	}
	return HWnD;

}

long long  CSwUtils::GetModelViewWindowHandle(BSTR fileName) {
	LOG_INFO("FileName = " << fileName);
	
	CComPtr<IModelDoc2> modelDoc = NULL;
	long long hwnd;
	if (SysStringLen(fileName) != 0) {
		modelDoc = GetModelDocByFileName(fileName);
	}
	else {
		userAddin->GetSldWorksPtr()->get_IActiveDoc2(&modelDoc);
	}

	if (modelDoc != NULL)
	{
		CComPtr<IModelView> modelView = NULL;
		modelDoc->get_IActiveView(&modelView);
		modelView->GetViewHWndx64(&hwnd);
		modelView.Release();
	}

	modelDoc.Release();
	return hwnd;

}


long long CSwUtils::GetMainFrameWindowHandle() {
	LOG_INFO("CSwUtils::GetModelViewWindowHandle called.");
	CComPtr<IFrame> iFrameObj;
	long long HWnD = 0;
	userAddin->GetSldWorksPtr()->IFrameObject(&iFrameObj);
	iFrameObj->GetHWndx64(&HWnD);
	return HWnD;
}


CComPtr<IModelDoc2> CSwUtils::GetModelDocByFileName(const wstring& fileName) {
	CComPtr<IModelDoc2> pModelDoc = NULL;
	CComBSTR bstrFileName((int)fileName.size(), fileName.data());
	userAddin->GetSldWorksPtr()->GetOpenDocument(bstrFileName,&pModelDoc );
	if (pModelDoc == NULL) {
		LOG_ERROR_FMT("GetOpenDocument('%s') Failed", fileName.c_str());
	}
	return pModelDoc;
}

bool CSwUtils::IsDocumentDirty(const wstring& fileName) {
	/*
	This flag:
	determines if the Do you wish to save changes? dialog is displayed when the user tries to close the document. Many operations cause this flag to be set, and you can use IModelDoc2::SetSaveFlag to set this flag.
	is set to true if the document was created in an older version of SOLIDWORKS.
	is set to true for assemblies only when a subassembly has been saved. If this flag is set to true for a subassembly, then the assembly is not marked as dirty until the subassembly is saved.
	is reset to false after an end-user has saved the file.
	*/
	CComPtr<IModelDoc2> pModelDoc = NULL;
	VARIANT_BOOL isDirty = VARIANT_FALSE;
	pModelDoc = GetModelDocByFileName(fileName);
	if (pModelDoc)
		pModelDoc->GetSaveFlag(&isDirty);

	if (!isDirty)
		return false;

	return true;

}

bool CSwUtils::SetFileAsReadOnly(const wstring& fileName){
	VARIANT_BOOL state = VARIANT_TRUE;
	VARIANT_BOOL success = VARIANT_FALSE;
	CComPtr<IModelDoc2> modelDoc = GetModelDocByFileName(fileName);
	
	if (modelDoc == NULL) {
		return false;
	}
	modelDoc->SetReadOnlyState(state, &success);
	if (!success) {
		return false;
	}
	return true;

}

bool CSwUtils::IsOpenedReadOnly(const wstring& fileName) {
	VARIANT_BOOL isReadOnly = VARIANT_FALSE;
	CComPtr<IModelDoc2> modelDoc = GetModelDocByFileName(fileName);

	if (modelDoc == NULL) {
		return false;
	}
	modelDoc->IsOpenedReadOnly(&isReadOnly);
	if (!isReadOnly) {
		return false;
	}
	return true;

}



void CSwUtils::GetModelViewsForModeDoc(CComPtr<IModelDoc2> modelDoc, vector<CComPtr<IModelView>> &modelViewList) {
	LOG_INFO("CSwUtils::GetModelViewsForModeDoc called");
	CComPtr<IEnumModelViews> pEnumModelView;
	CComPtr<IModelView> pModelView;
	long nFetched = -1;
	HRESULT hr = S_OK;
	modelDoc->EnumModelViews(&pEnumModelView);
	if (pEnumModelView) {
		pEnumModelView->Reset();
		do
		{
			pModelView = NULL;
			hr = pEnumModelView->Next(1, &pModelView, &nFetched);
			if (pModelView)
			{
				modelViewList.push_back(pModelView);
			}
		} while (pModelView);
	}

}

CComPtr<IModelDoc2> CSwUtils::GetModeDocForModelView(CComPtr<IModelView> modelView) {
	LOG_DEBUG("CSwUtils::GetModeDocForModelView called");
	vector<pair<CComPtr<IModelDoc2>, bool>> openedDocList;
	CComPtr<IModelDoc2> modelDoc = NULL;
	GetOpenedModelDocs(openedDocList);
	for (auto openedDoc : openedDocList) {
		modelDoc = openedDoc.first;
		vector<CComPtr<IModelView>> modelViewList;
		GetModelViewsForModeDoc(modelDoc,modelViewList);
		if (std::find(modelViewList.begin(), modelViewList.end(), modelView) != modelViewList.end()) {
			return modelDoc;
		}
	}
	return modelDoc;
}


long CSwUtils::GetExternalFileReferenceCount(CComPtr<IModelDoc2> modelDoc) {
	LOG_DEBUG("CSwUtils::GetExternalFileReferenceCount called");
	long nRefCount = 0;
	CComPtr<IModelDocExtension> modelDocExtn;
	if (modelDoc != NULL) {
		modelDoc->get_Extension(&modelDocExtn);
		if (modelDocExtn != NULL) {
			modelDocExtn->ListExternalFileReferencesCount(&nRefCount);
		}
	}
	LOG_DEBUG("RefCount = " << nRefCount);
	return nRefCount;
}


void CSwUtils::GetListExternalFileReferences(CComPtr<IModelDoc2> modelDoc , vector<ExternalFileReference> &extFileRefObjList) {
	LOG_DEBUG("CSwUtils::GetListExternalFileReferences called");
	CComPtr<IModelDocExtension> modelDocExtn;
	if (modelDoc == NULL)
		return;
	modelDoc->get_Extension(&modelDocExtn);
	if (modelDocExtn == NULL)
		return;
	long nRefCount = GetExternalFileReferenceCount(modelDoc);
	if (nRefCount >= 1) {
		BSTR *modelPathName = new BSTR[nRefCount];
		BSTR *compPathName = new BSTR[nRefCount]; // This contains the name of the part file from which part is derived.
		BSTR *feature = new BSTR[nRefCount];
		BSTR *dataType = new BSTR[nRefCount];
		long *status = new long[nRefCount];
		BSTR *refEntity = new BSTR[nRefCount];
		BSTR *featComp = new BSTR[nRefCount];
		long configOption;
		CComBSTR configName;

		modelDocExtn->IListExternalFileReferences(nRefCount, modelPathName, compPathName, feature, dataType, status, refEntity, featComp, &configOption, &configName);
		for (int i = 0; i < nRefCount; i++) {
			LOG_DEBUG_FMT("ModelPathName = %s , ComponentPathName = %s, Feature = %s, DataType = %s, Status = %d, ReferenceEntitiy = %s, FeatureComponent = %s, ConfigOption = %d, configName = %s",\
				modelPathName[i], compPathName[i], feature[i], dataType[i], status[i], refEntity[i], featComp[i], configOption,configName);
			ExternalFileReference extFileRefObj;
			extFileRefObj.modelPathName = modelPathName[i];
			SysFreeString(modelPathName[i]);
			extFileRefObj.compPathName = compPathName[i];
			SysFreeString(compPathName[i]);
			extFileRefObj.feature = feature[i];
			SysFreeString(feature[i]);
			extFileRefObj.dataType = dataType[i];
			SysFreeString(dataType[i]);
			extFileRefObj.status = status[i];
			extFileRefObj.refEntity = refEntity[i];
			SysFreeString(refEntity[i]);
			extFileRefObj.featComp = featComp[i];
			SysFreeString(featComp[i]);
			extFileRefObj.configOption = configOption;
			if(configName != NULL)
				extFileRefObj.configName = configName; 
			extFileRefObjList.push_back(extFileRefObj);

		}

		delete[] modelPathName;
		delete[] compPathName;
		delete[] feature;
		delete[] dataType;
		delete[] status;
		delete[] refEntity;
		delete[] featComp;
	}
}


void CSwUtils::GetReferenceComponentPathNames(CComPtr<IModelDoc2> modelDoc, vector<wstring> &compPathNameList) {
	LOG_INFO("CSwUtils::GetReferenceComponentNames called");
	vector<ExternalFileReference> extFileRefObjList;
	GetListExternalFileReferences(modelDoc, extFileRefObjList);
	for (auto extFileRef : extFileRefObjList) {
		compPathNameList.push_back(extFileRef.compPathName);
	}
}




bool CSwUtils::IsFileReferenceByModelDoc(const wstring &parentFileName, const wstring &referencedFileName) {
	LOG_INFO_FMT("ParentFileName = %s , ReferencedFileName = %s", parentFileName.c_str(), referencedFileName.c_str());
	vector<wstring> refFileNameList = GetReferencedModelDocuments(parentFileName);
	//Check if the fileName exactly matches one of the references in the vector referencedDocuments
	vector<wstring>::iterator  iter = std::find(refFileNameList.begin(), refFileNameList.end(), referencedFileName);
	if (iter != refFileNameList.end())
		return true;

	return false;
}


bool CSwUtils::GetSwUserPreferenceStringValue(swUserPreferenceStringValue_e val, wstring &stringVal) {
	CComBSTR bstrVal;
	HRESULT status = userAddin->GetSldWorksPtr()->GetUserPreferenceStringValue(val, &bstrVal);
	if (status != S_OK) {
		LOG_ERROR("Error reading user preference string value. SwUserPreferenceStringValue" <<val);
		return false;
	}
	stringVal = bstrVal;
	LOG_DEBUG_FMT("SwUserPreferenceStringValue = %d , StringValueRead = %s", val, stringVal.c_str());
	
	return true;
}


bool CSwUtils::IsWorkSpaceFolder(const wstring &dir) {
	wstring SW2CacheFolder = SwimUtils::GetSW2CacheFolder();
	if (SW2CacheFolder.empty())
		return false;

	if (dir.find(SW2CacheFolder) == std::wstring::npos)
		return false;

	wstring referencedDocumentsLoc = L"";
	if (!GetSwUserPreferenceStringValue(swFileLocationsDocuments, referencedDocumentsLoc))
		return false;
	
	if (!referencedDocumentsLoc.empty()) {
		vector<wstring> refDocLoc;
		FileUtils::wsTokenizer(referencedDocumentsLoc, L";", refDocLoc);
		for (const auto& loc : refDocLoc) {
			if (!_wcsicmp(loc.c_str(), dir.c_str()))
				return true;
		}
	}
	return false;
}


wstring CSwUtils::GetModelNameForModelDoc(CComPtr<IModelDoc2> modelDoc) {
	CComBSTR bstrModelName;
	wstring wsModelName = L"";
	if (modelDoc != NULL)
	{
		modelDoc->GetPathName(&bstrModelName);
	}
	wsModelName = bstrModelName;
	return wsModelName;
}

bool CSwUtils::IsNXLDocOpenedInMemory() {
	vector<pair<CComPtr<IModelDoc2>, bool>> openedFileDocs;
	GetOpenedModelDocs(openedFileDocs);
	for (auto openedDoc : openedFileDocs) {
		CComBSTR szPathName;
		openedDoc.first->GetPathName(&szPathName);
		if (szPathName.Length() > 0) {
			bool isProtected = false;
			if (CSwRMXMgr::GetInstance()->RMX_RPMGetFileStatus(szPathName.m_str, &isProtected) && isProtected) {
				return true;
			}
		}
	}
	return false;
}

bool CSwUtils::IsFileHasNXLDoc(const wstring& fileName) {

	CSwRMXMgr *swRMXMgrInstance = CSwRMXMgr::GetInstance();
	bool isProtected = false;

	swRMXMgrInstance->RMX_RPMGetFileStatus(fileName,&isProtected);
	if (isProtected)
		return true;

	long docType = CSwUtils::GetInstance()->GetModelDocTypeByName(fileName);
	if (docType == swDocNONE) {
		 docType = CSwUtils::GetInstance()->GetModelDocType();
	}

	if (docType == swDocASSEMBLY || docType == swDocDRAWING) {
		CComPtr<IModelDoc2> doc = CSwUtils::GetInstance()->GetModelDocByFileName(fileName);
		if (!doc) {
			doc = CSwUtils::GetInstance()->GetCurrentlyActiveModelDoc();
		}
		vector<wstring> referencedFileNames = CSwUtils::GetInstance()->GetReferencedFilesOfModelDoc(doc);
		for (auto refFileName : referencedFileNames) {
			swRMXMgrInstance->RMX_RPMGetFileStatus(refFileName, &isProtected);
			if (isProtected)
				return true;
		}
	}
	return false;
}



bool CSwUtils::FindSourceFile(const wstring& lpExistingFileName, const wstring& lpNewFileName, wstring& sourceDoc) {

	bool found = false;
	auto activeModelDoc = GetCurrentlyActiveModelDoc();

	//1. Check if lpNewFileName points to Auto-Recover file
	if (!found && SwFileTypes::IsAutoRecoverFile(lpNewFileName)) {
		wstring origDocForRecFile = L"";
		GetSrcDocForAutoRecoverFile(lpNewFileName, origDocForRecFile);
		if (!origDocForRecFile.empty()) {
			sourceDoc = origDocForRecFile;
			found = true;
		}
	}
	//2.Check if lpExistingFileName points to the already opened file in memory.
	if (!found) {
		if (IsFileOpened(lpExistingFileName) != NULL) {
			//We are more certain about the source doc at this moment. It happens with Pack And Go/BackUp Files.
			sourceDoc = lpExistingFileName;
			found = true;
		}
	}
	//3.Check if the file pointed by lpNewFileName is already opened in SolidWorks.
	if (!found) {
		if (IsFileOpened(lpNewFileName) != NULL) {
			//Control will reach here for File->Save/SaveAll operation.
			//lpExistingFileName=%tmp%\~0xE9FC.tmp , lpNewFileName=<working_dir>\sample.sldprt
			sourceDoc = lpNewFileName;
			found = true;
		}
	}
	//4.Check if the lpNewFileName is in the static vector CSldWorksRMX::TCDestSrcDocMap 
	if (!found) {
		if (CSldWorksRMX::TCDestSrcDocMap.size() > 0) {
			//TCDestSrcDocMap is filled for managed mode use cases like New/Replace
			int count = 0;
			for (auto pairObj : CSldWorksRMX::TCDestSrcDocMap) {
				if (!_wcsicmp(lpNewFileName.c_str(), pairObj.first.c_str())) {
					sourceDoc = pairObj.second;
					found = true;
					break;
				}
				++count;
			}
			if (found) {
				CSldWorksRMX::TCDestSrcDocMap.erase(CSldWorksRMX::TCDestSrcDocMap.begin() + count);
				if (IsFileOpened(sourceDoc) == NULL) {
					//Control will reach here when a new part file is opened and from TC taskpane , New is selected in the SaveAs dialog.
					sourceDoc = L"";
				}
			}
		}
	}
	//5. Check if the file being saved is derived component.
	if (!found
		&& activeModelDoc) {
		wstring activeDoc = GetModelNameForModelDoc(activeModelDoc);
		if (activeDoc.empty() && GetModelDocType() == swDocPART) {
			//Get the referenced component path names
			vector<wstring> compPathNameList;
			GetReferenceComponentPathNames(activeModelDoc, compPathNameList);
			//Check if any of the entry in compPathNameList matches with the reference of the assembly file.
			CComBSTR extRefName;
			activeModelDoc->GetExternalReferenceName(&extRefName); //Fetches the assembly file name.
			for (auto compFileName : compPathNameList) {
				bool isReference = IsFileReferenceByModelDoc(wstring(extRefName), compFileName);
				sourceDoc = compFileName;
				found = true;
				break;
			}
		}
	}
	//6. Check if the lpNewFileName points to file corresponding to the references of assembly/drawing
	if (!found
		&& activeModelDoc) {
		long fromDocType = 0;
		long toDocType = GetModelDocTypeByName(lpNewFileName);
		activeModelDoc->GetType(&fromDocType);

		//Get the source file for the referenced document in Assembly or Drawing when performing "Save As" that "Include all referenced components".
		if ((fromDocType == swDocASSEMBLY || fromDocType == swDocDRAWING) && (toDocType == swDocPART || toDocType == swDocASSEMBLY)) {
			//If referenced document is protected then protect the corresponding part file with the same level of protection as of the referenced document.
			//If referenced document is unprotected then skip protection for the destination file.
			const wstring& sourceFileName = GetSrcReferencedModelDoc(sourceDoc, lpNewFileName);
			if (!sourceFileName.empty()) {
				sourceDoc = sourceFileName;
				found = true;
			}
		}
	}
	//7.if still cannot find source file then finally assign source to the currently active document
	if (!found && activeModelDoc) {
		sourceDoc = GetModelNameForModelDoc(activeModelDoc);
		found = true;
	}
	LOG_INFO_FMT("lpExistingFileName = %s, lpNewFileName = %s, sourceDoc = %s , found = %d", lpExistingFileName.c_str(), lpNewFileName.c_str(), sourceDoc.c_str(), found);
	return found;
}

wstring CSwUtils::GetSrcReferencedModelDoc(wstring activeDoc, wstring fileName) {
	//Set sourceFile to activeDoc to handle case when assembly file is saved as part file or
	//assembly is exported as JT which belongs to the file itself instead of its references.
	wstring sourceFile = activeDoc;
	wstring destFileNameWithoutExtn = FileUtils::GetFileName(fileName);
	//JT translator replaces '-' with '_' while saving file to .jt format if the source file name contains '-'.
	if (SwFileTypes::IsJTFile(fileName)) {
		std::replace(destFileNameWithoutExtn.begin(), destFileNameWithoutExtn.end(), '_', '-');
	}

	vector<wstring> referencedDocuments = CSwUtils::GetInstance()->GetReferencedModelDocuments();

	//Check if the fileName exactly matches one of the references in the vector referencedDocuments
	for (auto refFileName : referencedDocuments) {
		if (!_wcsicmp(FileUtils::GetFileNameWithExt(refFileName).c_str(), FileUtils::GetFileNameWithExt(fileName).c_str())) {
			sourceFile = refFileName;
			return sourceFile;
		}

	}

	//find function used for assembly/drawing file when performing SaveAs->Include all referenced components->Add prefix/Add suffix
	size_t countCharNotMatched = UINT64_MAX;
	for (auto refFileName : referencedDocuments) {
		size_t tmpCountCharNotMatched = 0;
		wstring tmpRefFileName = FileUtils::GetFileName(refFileName);
		size_t index = destFileNameWithoutExtn.find(tmpRefFileName);
		if (index != std::wstring::npos) {
			//If suffix was applied, then index must always be equal to 0
			//If prefix was applied, then following must hold true : index+size(tmpRefFileName) == size(destFileNameWithoutExtn)
			if (index == 0 /*suffix*/ || (index + tmpRefFileName.size() == destFileNameWithoutExtn.size())/*prefix*/) {
				tmpCountCharNotMatched = destFileNameWithoutExtn.size() - tmpRefFileName.size();
				if (tmpCountCharNotMatched < countCharNotMatched) {
					sourceFile = refFileName;
					countCharNotMatched = tmpCountCharNotMatched;
				}
			}
		}
	}


	return sourceFile;
}

bool CSwUtils::GetSrcDocForAutoRecoverFile(wstring fileName, wstring & srcDoc) {
	//Auto recover file is of the form : "AutoRecover Of sample.SLDPRT.swar"
	wstring autoRecoverFileName = FileUtils::GetFileNameWithExt(fileName);
	vector<pair<CComPtr<IModelDoc2>, bool>> openedFileDocs;
	GetOpenedModelDocs(openedFileDocs);
	for (auto openedDoc : openedFileDocs) {
		CComBSTR szPathName;
		openedDoc.first->GetPathName(&szPathName);
		wstring openedFileName = FileUtils::GetFileNameWithExt(wstring(szPathName));
		size_t index = autoRecoverFileName.find(openedFileName);
		if (index != std::wstring::npos) {
			srcDoc = szPathName;
			LOG_INFO_FMT("FileName = %s ,SourceDoc = %s ", fileName.c_str(), srcDoc.c_str());
			
			return true;
		}
	}
	LOG_INFO("Source Doc couldn't be found for the file FileName =" << fileName);
	return false;
}
