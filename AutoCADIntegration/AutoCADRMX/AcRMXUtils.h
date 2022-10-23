#pragma once

class AcRMXUtils
{
public:
	static CString DocLockModeToStr(AcAp::DocLockMode eMode);
	static CString BooleanToStr(bool b);
	static CString		dbToStr(AcDbDatabase* pDB);
	static CString      objToClassStr(const AcRxObject* pObj);
	static CString      objToHandleStr(const AcDbObject* pObj);

	//
	// ACAD UI UTILITIES
	static void         PrintAcError(Acad::ErrorStatus eErrID);
	static void         AlertAcError(Acad::ErrorStatus eErrID);
	static const TCHAR* GetAcErrorMessage(Acad::ErrorStatus eErrID);

	//
	// ACAD Specific
	static bool IsDocModified();
	static bool IsSupportedFileType(LPCTSTR pszFileName);
	static bool GetSysVar(LPCTSTR pszVarName, CString& strVal);
	static bool GetSysVar(LPCTSTR pszVarName, int& nVal);

	static AcDbDictionary* OpenDictionaryForRead(LPCTSTR pszDictName, AcDbDatabase* pDB);
	static AcDbDictionary* OpenDictionaryForRead(const AcDbObjectId& dictId, AcDbDatabase* pDB);
	static AcDbDictionary* OpenDictionaryForRead(LPCTSTR pszDictName, AcDbDictionary* pParentDict);

	//
	// UI Utlitities
	static void    AlertWarn(LPCTSTR pszMsg);

	static void    AlertError(LPCTSTR pszMsg);
	static void    AlertError(UINT nID);

	static void    AlertInfo(LPCTSTR pszMsg);

	static CString LoadString(UINT nID);
	static CString FormatString(UINT nFormatID, ...);

	// Xref
	template<typename ApplyAction>
	static bool TraverseXrefsForCurrentDoc(const ApplyAction& actionFunc)
	{
		AcDbXrefGraph graph;
		acedGetCurDwgXrefGraph(graph);
		// No xref
		if (graph.numNodes() <= 1)
			return false;

		return TraverseXrefsGraph(graph.hostDwg(), actionFunc);
	}

	template<typename ApplyAction>
	static bool TraverseXrefs(const ApplyAction& actionFunc, AcDbDatabase* pDB = nullptr /*Current dwg when it not specified*/)
	{
		AcDbXrefGraph graph;
		if (pDB != nullptr)
			acdbGetHostDwgXrefGraph(pDB, graph);
		else
			acedGetCurDwgXrefGraph(graph);

		// No xref
		if (graph.numNodes() <= 1)
			return false;

		return TraverseXrefsGraph(graph.hostDwg(), actionFunc);
	}

	template<typename ApplyAction>
	static bool TraverseXrefsGraph(AcDbXrefGraphNode* pParentNode, const ApplyAction& actionFunc)
	{
		if (pParentNode == NULL || pParentNode->numOut() == 0)
			return false;

		AcDbXrefGraphNode* pNode = NULL;
		for (int i = 0; i < pParentNode->numOut(); i++)
		{
			pNode = (AcDbXrefGraphNode*)pParentNode->out(i);
			if (pNode != NULL && pNode->xrefStatus() == AcDb::kXrfResolved)
			{
				const TCHAR* szXrefFullPath = acdbOriginalXrefFullPathFor(pNode->database());
				bool abort = actionFunc(szXrefFullPath);
				if (abort) return true;

				abort = TraverseXrefsGraph(pNode, actionFunc);
				if (abort) return true;
			}
		}

		return false;
	}

	template<typename ApplyAction>
	static bool WalkRefDictionaryForRead(LPCTSTR dictName, AcDbDatabase* pDB, const ApplyAction& actionFunc)
	{
		if (pDB == NULL)
			return false;

		AcDbDictionary* pDict = AcRMXUtils::OpenDictionaryForRead(dictName, pDB);
		if (pDict == NULL)
			return false;

		// Run through the entries and call the template function.
		//
		Acad::ErrorStatus es;
		AcDbDictionaryIterator* pDictItr = pDict->newIterator();
		for (; !pDictItr->done(); pDictItr->next()) {
			AcDbObject* pObj = NULL;
			es = pDictItr->getObject(pObj, AcDb::kForRead);
			// if ok
			if (es == Acad::eOk)
			{
				AcString fileFullPath;
				// get the filepath for this reference
				if (pObj->isKindOf(AcDbUnderlayDefinition::desc()))
				{
					AcDbUnderlayDefinition* pUnderlayDef = AcDbUnderlayDefinition::cast(pObj);
					if (pUnderlayDef && pUnderlayDef->isLoaded())
					{
						AcString underlayFileName;
						auto es = pUnderlayDef->getActiveFileName(underlayFileName);
						// Any file does not have granted right, stop travering and return.
						if (es == Acad::eOk)
							fileFullPath = underlayFileName;
					}
				}
				else if (pObj->isKindOf(AcDbRasterImageDef::desc()))
				{
					AcDbRasterImageDef* pImageDef = AcDbRasterImageDef::cast(pObj);
					if (pImageDef && pImageDef->isLoaded())
					{
						fileFullPath = pImageDef->activeFileName();
					}
				}
				else if (pObj->isKindOf(AcDbPointCloudDefEx::desc()))
				{
					AcDbPointCloudDefEx* pDef = AcDbPointCloudDefEx::cast(pObj);
					if (pDef && pDef->isLoaded())
					{
						fileFullPath = pDef->activeFileName();
					}
				}
				bool abort = actionFunc(fileFullPath.constPtr());
				if (abort) return true;
				
			}
		}
		delete pDictItr;

		pDict->close();

		return false;
	}

	template<typename ApplyAction>
	static bool WalkRefDictionaryForRead(const AcDbObjectId& dictId, AcDbDatabase* pDB, const ApplyAction& actionFunc)
	{
		if (dictId == 0 || pDB == NULL)
			return false;

		AcDbDictionary* pDict = AcRMXUtils::OpenDictionaryForRead(dictId, pDB);
		if (pDict == NULL)
			return false;

		// Run through the entries and call the template function.
		//
		Acad::ErrorStatus es;
		AcDbDictionaryIterator* pDictItr = pDict->newIterator();
		for (; !pDictItr->done(); pDictItr->next()) {
			AcDbObject* pObj = NULL;
			es = pDictItr->getObject(pObj, AcDb::kForRead);
			// if ok
			if (es == Acad::eOk)
			{
				AcString fileFullPath;
				// get the filepath for this reference
				if (pObj->isKindOf(AcDbUnderlayDefinition::desc()))
				{
					AcDbUnderlayDefinition* pUnderlayDef = AcDbUnderlayDefinition::cast(pObj);
					if (pUnderlayDef && pUnderlayDef->isLoaded())
					{
						AcString underlayFileName;
						auto es = pUnderlayDef->getActiveFileName(underlayFileName);
						// Any file does not have granted right, stop travering and return.
						if (es == Acad::eOk)
							fileFullPath = underlayFileName;
					}
				}
				else if (pObj->isKindOf(AcDbRasterImageDef::desc()))
				{
					AcDbRasterImageDef* pImageDef = AcDbRasterImageDef::cast(pObj);
					if (pImageDef && pImageDef->isLoaded())
					{
						fileFullPath = pImageDef->activeFileName();
					}
				}
				else if (pObj->isKindOf(AcDbPointCloudDefEx::desc()))
				{
					AcDbPointCloudDefEx* pDef = AcDbPointCloudDefEx::cast(pObj);
					if (pDef && pDef->isLoaded())
					{
						fileFullPath = pDef->activeFileName();
					}
				}
				bool abort = actionFunc(fileFullPath.constPtr());
				if (abort) return true;
			}
		}
		delete pDictItr;

		pDict->close();

		return false;
	}


	template<typename ApplyAction>
	static bool TraverseUnderlayRefsForCurrent(const ApplyAction& actionFunc)
	{
		// Pdf underlay
		auto pdfDictName = AcDbUnderlayDefinition::dictionaryKey(AcDbPdfDefinition::desc());
		if (WalkRefDictionaryForRead(pdfDictName, acdbCurDwg(), actionFunc))
			return true;

		// Dgn underlay
		auto dgnDictName = AcDbUnderlayDefinition::dictionaryKey(AcDbDgnDefinition::desc());
		if (WalkRefDictionaryForRead(dgnDictName, acdbCurDwg(), actionFunc))
			return true;

		// dwf underlay
		auto dwfDictName = AcDbUnderlayDefinition::dictionaryKey(AcDbDwfDefinition::desc());
		if (WalkRefDictionaryForRead(dwfDictName, acdbCurDwg(), actionFunc))
			return true;

		// Image underlay
		auto imageDictId = AcDbRasterImageDef::imageDictionary(acdbCurDwg());
		if (WalkRefDictionaryForRead(imageDictId, acdbCurDwg(), actionFunc))
			return true;

		auto pcDictId = AcDbPointCloudDefEx::pointCloudExDictionary(acdbCurDwg());
		if (WalkRefDictionaryForRead(pcDictId, acdbCurDwg(), actionFunc))
			return true;

		return false;
	}

	template<typename ApplyAction>
	static bool TraverseUnderlayRefs(const ApplyAction& actionFunc, AcDbDatabase* pDB = nullptr /*Current dwg when it not specified*/)
	{	
		AcDbDatabase* pDBHost = pDB;
		if (pDBHost == nullptr)
			pDBHost = acdbCurDwg();

		// Pdf underlay
		auto pdfDictName = AcDbUnderlayDefinition::dictionaryKey(AcDbPdfDefinition::desc());
		if (WalkRefDictionaryForRead(pdfDictName, pDBHost, actionFunc))
			return true;

		// Dgn underlay
		auto dgnDictName = AcDbUnderlayDefinition::dictionaryKey(AcDbDgnDefinition::desc());
		if (WalkRefDictionaryForRead(dgnDictName, pDBHost, actionFunc))
			return true;

		// dwf underlay
		auto dwfDictName = AcDbUnderlayDefinition::dictionaryKey(AcDbDwfDefinition::desc());
		if (WalkRefDictionaryForRead(dwfDictName, pDBHost, actionFunc))
			return true;

		// Image underlay
		auto imageDictId = AcDbRasterImageDef::imageDictionary(pDBHost);
		if (WalkRefDictionaryForRead(imageDictId, pDBHost, actionFunc))
			return true;

		auto pcDictId = AcDbPointCloudDefEx::pointCloudExDictionary(pDBHost);
		if (WalkRefDictionaryForRead(pcDictId, pDBHost, actionFunc))
			return true;

		return false;
	}
	
	// Others
	static void PrintAllCommands();
};

