#pragma once
#include "CatEventMgr.h"

struct TCatRMXNxlFile
{
	wstring m_wsFileFullName;
	string m_strTags;
	DWORD m_dwRights;
};

class CCatSession : public RMXControllerBase<CCatSession>, public CCatEventLisener
{
public:
	static bool GetDocFileName(CAT::DocumentPtr pDoc, wstring& retFileFullName);

public:
	void Start();
	void Stop();

	//
	// Session accessors
	CAT::ApplicationPtr GetCatApp() { return m_pCatApp; }
	bool IsConnected() const { return (m_pCatApp != NULL); }
	wstring GetInstallDir() const { return m_wsAppInstallDir; }
	HWND GetMainWnd() const;

	//
	// Doc access
	//
	CAT::DocumentPtr GetActiveDocument();
	CAT::WindowPtr GetActiveWindow();
	CAT::DocumentPtr FindDocument(const wstring& wsFileFullName);

	//
	// NXL file functions
	//
	bool AddNxlFile(const wstring& wsFileFullName);
	void RemoveNxlFile(const wstring& wsFileFullName);
	void RefreshNxlCache();

	bool IsNxlFile(const wstring& wsFileFullName) const;
	bool IsNxlFile(CAT::DocumentPtr pDoc) const;

	string GetTags(const wstring& wsFileFullName) const;

	bool CheckRight(const wstring& wsFileFullName, DWORD dwRights, bool audit = true) const;

	// To optimize performance, return once any xref does not have permission. 
	// Will not scan all xrefs, so the return set always contains one element now.
	//
	//template<typename ApplyAction>
	//bool TraverseActiveDoc(CAT::DocumentPtr pParentDoc, const ApplyAction& actionFunc)
	//{
	//	CHK_ERROR_RETURN(!m_pCatApp, false, L"Traverse terminated (error %s)", L"Catia session not connected");

	//	CAT::DocumentPtr pRootDoc;
	//	m_pCatApp->get_ActiveDocument(&pRootDoc);
	//	CHK_ERROR_RETURN(!pRootDoc, false, L"Traverse terminated (error %s)", L"Active document not found");
	//
	//	return TraverseParent(pRootDoc, actionFunc);
	//}

	//template<typename ApplyAction>
	//bool TraverseParent(CAT::DocumentPtr pParentDoc, const ApplyAction& actionFunc)
	//{
	//	CComBSTR bstrFileName;
	//	pParentDoc->get_FullName(&bstrFileName);
	//	wstring wsFileName = StringHlp::bstr2ws(bstrFileName.m_str);
	//	
	//	StringHlp::ToLower<wchar_t>(wsFileName);

	//	const wstring wsProductExt(L".catproduct");
	//	if (std::equal(wsProductExt.rbegin(), wsProductExt.rend(), wsFileName.rbegin())) 
	//	{
	//		CAT::ProductDocumentPtr pProductDoc = (CAT::ProductDocumentPtr)pParentDoc;
	//		CAT::ProductPtr pRootProduct;
	//		if(FAILED(pProductDoc->get_Product(&pRootProduct)))
	//			return true;
	//		
	//		CAT::ProductsPtr pChildProducts;
	//		if (FAILED(pRootProduct->get_Products(&pChildProducts)))
	//			return true;
	//			
	//		long lNbItems = 0;
	//		pChildProducts->get_Count(&lNbItems);
	//		for (long i = 1; i <= lNbItems; i++)
	//		{
	//			CComVariant vIndex(i);
	//			CAT::ProductPtr pChildProduct;
	//			if (FAILED(pChildProducts->raw_Item(&vIndex, &pChildProduct)))
	//				continue;
	//			CAT::ProductPtr pRefProduct;
	//			if (FAILED(pChildProduct->get_ReferenceProduct(&pRefProduct)))
	//				continue;
	//						
	//			CAT::CATBaseDispatchPtr pChildDocTmp;
	//			if (FAILED(pRefProduct->get_Parent(&pChildDocTmp)))
	//				continue;
	//				
	//			CAT::DocumentPtr pChildDoc = (CAT::DocumentPtr)pChildDocTmp;

	//			bool abort = actionFunc(pChildDoc);
	//			if (abort) return true;

	//			abort = TraverseParent(pChildDoc, actionFunc);
	//			if (abort) return true;
	//		}
	//	}
	//	return false;
	//}

	//
	// Traverse docs in Cat session
	// 
	template<typename ApplyAction>
	bool WalkOpenningDocs(const ApplyAction& actionFunc)
	{
		CAT::DocumentsPtr pDocs;
		HRESULT hr = m_pCatApp->get_Documents(&pDocs);
		if (FAILED(hr) || !pDocs)
		{
			LOG_ERROR(L"Cannot get documents");
			return false;
		}
		long lCount = 0;
		pDocs->get_Count(&lCount);
		if (lCount == 0)
		{
			LOG_DEBUG(L"No doc opened");
			return false;
		}

		LOG_DEBUG(L"Number of openning docs: " << lCount);
		for (long i = 1; i <= lCount; ++i)
		{
			CComVariant vIndex(i);
			CAT::DocumentPtr pDoc;
			pDocs->Item(&vIndex, &pDoc);
			if (pDoc && actionFunc(pDoc))
				return true; //Terminate loop
		}

		return false;
	}

	template<typename ApplyAction>
	bool TraverseSubLinks(CAT::StiDBItemPtr pDBItem, const ApplyAction& actionFunc, set<wstring, ICaseKeyLess>& processedLinks)
	{		
		CAT::StiDBChildrenPtr pDBChildren;
		if (FAILED(pDBItem->GetChildren(&pDBChildren)))
			return false;
		
		long lNbChildren = 0;
		pDBChildren->get_Count(&lNbChildren);

		wstring logMsg;
		bool abort = false;
		for (long i = 1; i <= lNbChildren; i++)
		{
			CComVariant vIndex(i);
			CAT::StiDBItemPtr pDBChild;
			if (FAILED(pDBChildren->Item(&vIndex, &pDBChild)) || !pDBChild)
				continue;

			CComBSTR bstrLinkType;
			pDBChildren->LinkType(&vIndex, &bstrLinkType);

			CComBSTR bstrFullPath;
			if(FAILED(pDBChild->GetDocumentFullPath(&bstrFullPath)))
				continue;

			const wstring& wsFileFullName = StringHlp::bstr2ws(bstrFullPath.m_str);

			// Some links found multiple times in the tree, for example, in drawing file, same link
			// can be retrieved for each view. We don't need to execute callback(ApplyAction) for serveral times one the same file. 
			// TODO: Investigate if any link type should be ignored. 
			if (processedLinks.find(wsFileFullName) == processedLinks.end())
			{
				processedLinks.insert(wsFileFullName);

				CAT::DocumentPtr pChildDoc;
				if(FAILED(pDBChild->GetDocument(&pChildDoc)) || !pChildDoc)
					continue;
				logMsg += StringHlp::FormatString(L"\n\t-[%ld][%s]%s processed", i, StringHlp::bstr2ws(bstrLinkType.m_str).c_str(), wsFileFullName.c_str());
				if (actionFunc(pChildDoc))
				{
					logMsg += L", ABORT loop!!!";
					abort = true;
					break; // Terminate loop
				}
			}

			if(TraverseSubLinks(pDBChild, actionFunc, processedLinks))
			{
				abort = true;
				break; // Terminate loop
			}
		}
		
		if (lNbChildren > 0)
		{
			CComBSTR bstrFullPath;
			pDBItem->GetDocumentFullPath(&bstrFullPath);
			logMsg.insert(0, StringHlp::FormatString(L"Walking thru sub-links (Parent: %s, Count: %ld)...", StringHlp::bstr2ws(bstrFullPath.m_str).c_str(), lNbChildren));
			LOG_DEBUG(logMsg.c_str());
		}

		return abort;
	}

	template<typename ApplyAction>
	bool TraverseLinks(CAT::DocumentPtr pParentDoc, const ApplyAction& actionFunc)
	{
		if (!pParentDoc)
			return false;

		CAT::CATBaseDispatchPtr pDisp;
		CComBSTR bstrSMEngine(L"CAIEngine");
		if (FAILED(m_pCatApp->GetItem(&bstrSMEngine.m_str, &pDisp)))
			return false;
		CAT::StiEnginePtr pSMEngine = (CAT::StiEnginePtr)pDisp;
		if (pSMEngine)
		{
			CAT::StiDBItemPtr pDBItem;
			if (FAILED(pSMEngine->GetStiDBItemFromAnyObject(pParentDoc, &pDBItem)))
				return false;

			set<wstring, ICaseKeyLess> processedLinks;
			return TraverseSubLinks(pDBItem, actionFunc, processedLinks);
		}

		return false;
	}

	// RMX Event handler
	void OnAfterCommand(LPCatCommandNotify pParam);
	void OnFrmLayoutChanged(LPCatFrmNotify pParam);

	//
	// Utility
	//
	static void DisplayAlert(UINT uMsBoxType, UINT nMsgFmtID, ...);
	void ShowStatusBar(UINT nMsgFmtID, ...);
	HWND GetFullScreenTopWindow() const;
	bool IsFullScreen() const;

private:
	CAT::ApplicationPtr m_pCatApp;
	std::map<std::wstring, TCatRMXNxlFile, ICaseKeyLess> m_nxlCacheMap;
	wstring m_wsAppDir;
	bool m_bAppUIMode;
	wstring m_wsAppInstallDir;
	HWND m_hMainWnd;
};

#define curCatSession CCatSession::GetInstance()
#define ALERT_ERROR(fmt, ...) CCatSession::DisplayAlert(MB_ICONERROR, fmt, __VA_ARGS__)
#define ALERT_WARN(fmt, ...) CCatSession::DisplayAlert(MB_ICONWARNING, fmt, __VA_ARGS__)
