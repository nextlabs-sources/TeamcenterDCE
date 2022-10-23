#include "StdAfx.h"
#include "AcInc.h"
#include "AcRMXTypes.h"
#include "AcRMXUtils.h"
#include <RMXTypes.h>

CString AcRMXUtils::DocLockModeToStr(AcAp::DocLockMode eMode)
{
	CString str;

	if (eMode & AcAp::kNone)
		str += L"None";

	if (!str.IsEmpty())
		str += L" | ";

	if (eMode & AcAp::kAutoWrite) {
		str += L"Auto Write";
	}

	if (eMode & AcAp::kNotLocked) {
		str += L"Not Locked";
	}

	if (eMode & AcAp::kWrite) {
		str +=  L"Write";
	}

	if (eMode & AcAp::kProtectedAutoWrite) {
		str += L"Protected Auto Write";
	}

	if (eMode & AcAp::kRead) {
		str += L"Read";
	}

	if (eMode & AcAp::kXWrite) {
		str += L"X Write";
	}

	return str;
}

CString AcRMXUtils::BooleanToStr(bool b)
{
	return b? L"True" : L"False";
}

CString AcRMXUtils::dbToStr(AcDbDatabase* pDb)
{
	CString str;

	if (pDb == NULL) {
		str = _T("NULL");
		return str;
	}

	const wchar_t* szFName;
	Acad::ErrorStatus eErrorStatus = pDb->getFilename(szFName);
	if (eErrorStatus == Acad::eOk)
		str.Format(_T("1. %p  \"%s\""), pDb, szFName);
	else {
		// see if we can get name from a document
		AcApDocument* pDoc = acDocManager->document(pDb);
		if (pDoc) {
			str.Format(_T("2. %p  \"%s\""), pDb, pDoc->fileName());
		}
		else {
			// last resort, just use the pointer value.  eNotApplicable
			// happens frequently on temporary databases, otherwise we
			// would like to know what is going on.
			if (eErrorStatus == Acad::eNotApplicable)
				str.Format(_T("%p"), pDb);
			else {
				ASSERT(0);
				str.Format(_T("%p  %s"), pDb, AcRMXUtils::GetAcErrorMessage(eErrorStatus));
			}
		}
	}

	return str;
}

CString AcRMXUtils::objToClassStr(const AcRxObject* pObj)
{
	ASSERT(pObj != NULL);
	if (pObj == NULL) {
		return  _T("NULL");
	}
	AcRxClass* pRxClass = pObj->isA();
	if (pRxClass == NULL) {
		ASSERT(0);
		AcRMXUtils::AlertError(_T("AcRxObject class has not called rxInit()!"));
		return _T("*Unknown*");
	}

	return pObj->isA()->name();
}

CString AcRMXUtils::objToHandleStr(const AcDbObject* pObj)
{
	ASSERT(pObj != NULL);
	if (pObj == NULL) {
		return  _T("NULL");
	}
	CString str;
	AcDbHandle handle;
	pObj->getAcDbHandle(handle);
	TCHAR szTmp[256];
	handle.getIntoAsciiBuffer(szTmp);
	str = szTmp;
	
	return str;
}

bool AcRMXUtils::IsDocModified()
{
	int val;
	if (GetSysVar(/*MSGO*/L"DBMOD", val) && val != 0)
		return true;


	return false;
}

bool AcRMXUtils::IsSupportedFileType(LPCTSTR pszFileName)
{
	CPathEx fPath(pszFileName);
	std::wstring& ext = fPath.GetExtension();
	if (ext.empty() || (wcsicmp(ext.c_str(), NXL_FILE_EXT_WITHOUT_DOT) == 0))
		return false;

	// Remove the "." header
	if (ext.front() == L'.')
		ext.erase(0, 1);

	return (AcRMX::SUPPORTED_FILETYPE_TABLE.find(ext) != AcRMX::SUPPORTED_FILETYPE_TABLE.end());
}

bool AcRMXUtils::GetSysVar(LPCTSTR pszValName, CString& strVal)
{
	resbuf rb;
	if (acedGetVar(pszValName, &rb) == RTNORM) {
		ASSERT(rb.restype == RTSTR);
		strVal = rb.resval.rstring;
		free(rb.resval.rstring);
		return true;
	}
		
	return false;
}

bool AcRMXUtils::GetSysVar(LPCTSTR pszVarName, int& nVal)
{
	resbuf rb;
	if (acedGetVar(pszVarName, &rb) == RTNORM) {
		ASSERT(rb.restype == RTSHORT);
		nVal = rb.resval.rint;
		return true;
	}

	return false;
}

AcDbDictionary* AcRMXUtils::OpenDictionaryForRead(LPCTSTR pszDictName, AcDbDatabase* pDB)
{
	ASSERT(pDB != NULL);

	// get the root dict for this database
	AcDbDictionary* pDict = NULL;
	AcDbDictionary* pRootDict;
	Acad::ErrorStatus es = pDB->getNamedObjectsDictionary(pRootDict, AcDb::kForRead);
	if (es == Acad::eOk) {
		// now get the sub-dict
		pDict = OpenDictionaryForRead(pszDictName, pRootDict);
		pRootDict->close();
	}
	else {
		LOG_ERROR_FMT(L"ARX ERROR: %s)", acadErrorStatusText(es));
	}

	return pDict;
}

AcDbDictionary* AcRMXUtils::OpenDictionaryForRead(const AcDbObjectId& dictId, AcDbDatabase* pDB)
{
	ASSERT(pDB != NULL);

	// get the root dict for this database
	AcDbDictionary* pDict = NULL;
	AcDbDictionary* pRootDict;
	Acad::ErrorStatus es = pDB->getNamedObjectsDictionary(pRootDict, AcDb::kForRead);
	if (es == Acad::eOk)
	{
		// now get the sub-dict
		AcString dictName;
		es = pRootDict->nameAt(dictId, dictName);
		if (es == Acad::eOk)
		{
			pDict = OpenDictionaryForRead(dictName.constPtr(), pRootDict);
		}
		pRootDict->close();
	}
	else {
		LOG_ERROR_FMT(L"ARX ERROR: %s)", acadErrorStatusText(es));
	}

	return pDict;
}

AcDbDictionary* AcRMXUtils::OpenDictionaryForRead(LPCTSTR pszDictName, AcDbDictionary* pParentDict)
{
	ASSERT(pszDictName != NULL);
	ASSERT(pParentDict != NULL);

	// get the dictionary that we are looking for
	AcDbObject* obj;
	Acad::ErrorStatus es = pParentDict->getAt(pszDictName, obj, AcDb::kForRead);

	if (es == Acad::eOk)
		return AcDbDictionary::cast(obj);
	else
		return NULL;
}

void AcRMXUtils::PrintAcError(Acad::ErrorStatus eErrID)
{
	if (eErrID != Acad::eOk)
		acutPrintf(_T("\nARX ERROR: %s"), acadErrorStatusText(eErrID));
}

void AcRMXUtils::AlertAcError(Acad::ErrorStatus eErrID)
{
	if (eErrID != Acad::eOk) {
		std::wstring str(L"ARX ERROR : ");
		str += acadErrorStatusText(eErrID);
		AcRMXUtils::AlertError(str.c_str());
	}
}

const TCHAR* AcRMXUtils::GetAcErrorMessage(Acad::ErrorStatus eErrID)
{
	return acadErrorStatusText(eErrID);
}

void AcRMXUtils::AlertWarn(LPCTSTR pszMsg)
{
	MessageBox(adsw_acadMainWnd(), pszMsg, ACRMX_MESSAGEDLG_LABEL, MB_ICONEXCLAMATION);
}

void AcRMXUtils::AlertError(LPCTSTR pszMsg)
{
	MessageBox(adsw_acadMainWnd(), pszMsg, ACRMX_MESSAGEDLG_LABEL, MB_ICONSTOP);
}

void AcRMXUtils::AlertError(UINT nID)
{
	const CString& msg = AcRMXUtils::LoadString(nID);
	AlertError(msg);
}

void AcRMXUtils::AlertInfo(LPCTSTR pszMsg)
{
	MessageBox(adsw_acadMainWnd(), pszMsg, ACRMX_MESSAGEDLG_LABEL, MB_ICONINFORMATION);
}

CString AcRMXUtils::LoadString(UINT nID)
{
	CAcModuleResourceOverride resOverride;

	CString ret;
	if (!ret.LoadString(nID))
	{
		LOG_ERROR_FMT(L"%d resource string not loaded", nID);
	}

	return ret;
}

CString AcRMXUtils::FormatString(UINT nFormatID, ...)
{
	CAcModuleResourceOverride resOverride;

	CString strFmt;
	if (!strFmt.LoadString(nFormatID))
	{
		LOG_ERROR_FMT(L"%d resource string not loaded", nFormatID);
		return CString(L"");
	}

	va_list vl;
	va_start(vl, nFormatID);

	CString ret;
	ret.FormatMessageV(strFmt, &vl);

	va_end(vl);
	
	return ret;
} 

void AcRMXUtils::PrintAllCommands()
{
	AcEdCommandIterator* iter = acedRegCmds->iterator();
	if (iter == NULL)
	{
		return;
	}
	CPathEx outPath = RMXUtils::getEnv(L"TEMP").c_str();
	outPath /= L"AcCommands.txt";
	int n = 0;
	FILE* fptr;
	_wfopen_s(&fptr, outPath.c_str(), L"a");
	if (fptr != NULL)
	{
		for (; !iter->done(); iter->next())
		{
			const AcEdCommand* pp = iter->command();
			fwprintf(fptr, _T("%s\n"), pp->globalName());
			n++;
		}

		fclose(fptr);
	}
}
