#pragma once

#include "stdafx.h"
#include "SldWorksRMX.h"

struct ExternalFileReference {
	wstring modelPathName;
	wstring compPathName;
	wstring feature;
	wstring dataType;
	long status;
	wstring refEntity;
	wstring featComp;
	long configOption;
	wstring configName;
};

class CSwUtils {

private:
	static CSwUtils * m_SwUtilsInstance;
	static CSldWorksRMX *userAddin;
	CSwUtils();
	~CSwUtils();

public:
	CSwUtils(const CSwUtils &) = delete;
	CSwUtils & operator=(const CSwUtils &) = delete;
	static CSwUtils * GetInstance();
	static CSldWorksRMX* GetUserAddin() {
		return userAddin;
	}
	static void Init(CSldWorksRMX* addinPtr) {
		userAddin = addinPtr;
	}
	vector<wstring> GetReferencedModelDocuments(wstring fileName = L"");
	vector<wstring> GetReferencedFilesOfModelDoc(CComPtr<IModelDoc2> &modeldoc);
	long CSwUtils::ShowMessageDialog(const wstring& message, long icons, long buttons);
	long GetModelDocType();
	long GetModelDocTypeByName(const wstring& fileName);
	wstring GetCurrentActiveFile();//TODO:to be deprecated in future
	CComPtr<IModelDoc2> GetCurrentlyActiveModelDoc();
	void GetOpenedModelDocs(vector<pair<CComPtr<IModelDoc2>, bool>> & openedFileList);
	void ListOpenedModelDocs();
	bool IsFileVisible(BSTR fileName);
	int GetVisibleDocumentsCount();
	bool IsLastVisibleDocument(const wstring& fileName);
	CComPtr<IModelDoc2> IsFileOpened(const wstring &filePath);
	CComPtr<IModelDoc2> IsFileOpened2(const wstring &fileName);
	long long  GetModelWindowHandle(BSTR fileName);
	long long  GetModelViewWindowHandle(BSTR fileName);
	long long GetMainFrameWindowHandle();
	wstring GetModelDocTypeByEnum(long type);
	bool IsDocumentDirty(const wstring& fileName);
	CComPtr<IModelDoc2> GetModelDocByFileName(const wstring& fileName);
	bool SetFileAsReadOnly(const wstring& fileName);
	static void ReleaseInstance() {
		LOG_INFO("CSwUtils::ReleaseInstance");
		if (m_SwUtilsInstance != nullptr) {
			userAddin = nullptr;
			delete m_SwUtilsInstance;
			m_SwUtilsInstance = nullptr;
		}
	}

	void GetModelViewsForModeDoc(CComPtr<IModelDoc2> modelDoc,vector<CComPtr<IModelView>> &modelViewList);
	CComPtr<IModelDoc2> GetModeDocForModelView(CComPtr<IModelView> modelView);
	bool IsVirtualModel(CComPtr<IModelDoc2> &modelDoc);
	bool TryFindRootFileOfVirtualModel(CComPtr<IModelDoc2>& modelDoc, wstring& rootFile=wstring(L""));
	long GetExternalFileReferenceCount(CComPtr<IModelDoc2> modelDoc);
	void GetListExternalFileReferences(CComPtr<IModelDoc2> modelDoc, vector<ExternalFileReference> &extFileRefObj);
	void GetReferenceComponentPathNames(CComPtr<IModelDoc2> modelDoc, vector<wstring> &compPathNameList);
	bool IsFileReferenceByModelDoc(const wstring &parentFileName, const wstring &referencedFileName);
	bool GetSwUserPreferenceStringValue(swUserPreferenceStringValue_e val, wstring &stringVal);
	wstring GetModelNameForModelDoc(CComPtr<IModelDoc2> modelDoc);
	bool IsNXLDocOpenedInMemory();
	bool IsOpenedReadOnly(const wstring& fileName);
	bool IsWorkSpaceFolder(const wstring &dir);
	bool IsFileHasNXLDoc(const wstring& fileName);
	bool FindSourceFile(const wstring& lpExistingFileName, const wstring& lpNewFileName, wstring& sourceDoc);
	wstring GetSrcReferencedModelDoc(wstring activeDoc, wstring fileName);
	bool GetSrcDocForAutoRecoverFile(wstring fileName, wstring & srcDoc);
};

#define theSwUtils CSwUtils::GetInstance()