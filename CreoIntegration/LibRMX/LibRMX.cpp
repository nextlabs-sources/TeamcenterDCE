// LibRMX.cpp : Defines the exported functions for the DLL application.
//
#include "LibRMX.h"

#include <string>
#include <tuple>

#include "..\common\CreoRMXLogger.h"
#include "RMXInstance.h"
#include "RMXMgr.h"
#include "RMXNxlFile.h"
#include "RMXResult.h"
#include <CADRMXTypeDef.h>
#include <RPMCommonDialogWrapper.hpp>

using namespace std;

LIBRMXAPI bool RMX_Initialize()
{
	return CRMXInstance::GetInstance().Initialize();
}

LIBRMXAPI void RMX_Terminate()
{
	CRMXInstance::GetInstance().Terminate();
}

LIBRMXAPI bool RMX_ProtectFile(const wchar_t* filePath, const char* tags, const wchar_t* newFilePath)
{
	const wstring wstrFilePath(filePath);
	const wstring wstrNewFilePath(newFilePath);
	return CRMXMgr::GetInstance().ProtectFile(tags != nullptr ? string(tags) : string(""), wstrFilePath, wstrNewFilePath);
}

LIBRMXAPI bool RMX_IsProtectedFile(const wchar_t * nxlFilePath)
{
	if (nxlFilePath == nullptr)
		return false;
	CRMXNxlFile nxlFile(nxlFilePath);
	return nxlFile.IsProtected();
}

LIBRMXAPI void RMX_GetTags(const wchar_t * nxlFilePath, char ** ppTags)
{
	if (nxlFilePath == nullptr || ppTags == nullptr)
		return;
	CRMXNxlFile nxlFile(nxlFilePath);
	string retTags = nxlFile.GetTags();
	if (!retTags.empty())
	{
		*ppTags = new char[retTags.length() + 1];
		strcpy(*ppTags, retTags.c_str());
	}
}

LIBRMXAPI bool RMX_IsRPMFolder(const wchar_t * dirPath)
{
	if (dirPath == nullptr)
		return false;
	const wstring wstrDirPath(dirPath);	
	RMX_SAFEDIRRELATION dirRel = RMX_NoneOfSafeDir;
	return CRMXMgr::GetInstance().IsRPMFolder(wstrDirPath);
}

LIBRMXAPI bool RMX_GetSafeDirRelation(const wchar_t * dirPath, RMX_SAFEDIRRELATION & dirRelation)
{
	if (dirPath == nullptr)
		return false;
	const wstring wstrDirPath(dirPath);
	return CRMXMgr::GetInstance().IsRPMFolder(wstrDirPath, &dirRelation);
}

LIBRMXAPI bool RMX_AddRPMDir(const wchar_t * dirPath)
{
	if (dirPath == nullptr)
		return false;

	const wstring wstrDirPath(dirPath);
	return CRMXMgr::GetInstance().AddRPMDir(wstrDirPath);
}

LIBRMXAPI bool RMX_AddRPMDir2(const wchar_t * dirPath, unsigned int option)
{
	if (dirPath == nullptr)
		return false;

	const wstring wstrDirPath(dirPath);
	return CRMXMgr::GetInstance().AddRPMDir(wstrDirPath, option);
}

LIBRMXAPI bool RMX_RemoveRPMDir(const wchar_t * dirPath)
{
	if (dirPath == nullptr)
		return false;

	const wstring wstrDirPath(dirPath);
	return CRMXMgr::GetInstance().RemoveRPMDir(wstrDirPath);
}

LIBRMXAPI bool RMX_CopyNxlFile(const wchar_t * srcNxlFilePath, const wchar_t * destNxlFilePath, bool deleteSource)
{
	if (srcNxlFilePath == nullptr || destNxlFilePath == nullptr)
		return false;

	CRMXNxlFile srcNxlFile(srcNxlFilePath);
	const wstring wstrDestFile(destNxlFilePath);
	return srcNxlFile.CopyTo(wstrDestFile, deleteSource);
}

LIBRMXAPI bool RMX_RPMResisterApp(const wchar_t * appPath)
{
	if (appPath == nullptr)
		return false;

	const wstring wstrAppPath(appPath);
	return CRMXMgr::GetInstance().RPMRegisterApp(wstrAppPath);
}

LIBRMXAPI bool RMX_RPMUnresisterApp(const wchar_t * appPath)
{
	if (appPath == nullptr)
		return false;

	const wstring wstrAppPath(appPath);
	return CRMXMgr::GetInstance().RPMUnregisterApp(wstrAppPath);
}

LIBRMXAPI bool RMX_RPMAddTrustedProcess(unsigned long processId)
{
	return CRMXMgr::GetInstance().RPMAddTrustedProcess(processId);
}

LIBRMXAPI bool RMX_CheckRights(const wchar_t * nxlFilePath, unsigned long rights, bool audit)
{
	if (nxlFilePath == nullptr)
		return false;

	CRMXNxlFile nxlFile(nxlFilePath);
	return nxlFile.CheckRights(rights, audit);
}

LIBRMXAPI bool RMX_DeleteNxlFile(const wchar_t * nxlFilePath)
{
	if (nxlFilePath == nullptr)
		return false;

	CRMXNxlFile nxlFile(nxlFilePath);
	return nxlFile.Delete();
}

LIBRMXAPI void RMX_GetRightName(unsigned long rights, wchar_t* name)
{
	const wstring wstrName = CRMXNxlFile::GetRightName(rights);
	wcscpy(name, wstrName.c_str());
}

LIBRMXAPI void RMX_ReleaseArray(void * pArr)
{
	if (pArr)
	{
		delete[] pArr;
		pArr = nullptr;
	}
}

LIBRMXAPI unsigned long RMX_GetLastError(wchar_t ** ppErrMsg)
{
	const CRMXResult& err = CRMXInstance::GetInstance().GetLastError();
	if (ppErrMsg)
	{
		const wstring& errMsg = err.ToString();
		if (!errMsg.empty())
		{
			*ppErrMsg = new wchar_t[errMsg.length() + 1];
			wcscpy(*ppErrMsg, errMsg.c_str());
		}
	}
	return err.GetCode();
}

LIBRMXAPI bool RMX_IsValidNxl(const wchar_t * nxlFilePath)
{
	if (nxlFilePath == nullptr)
		return false;
	CRMXNxlFile nxlFile(nxlFilePath);
	return nxlFile.IsValidNxl();
}

LIBRMXAPI bool RMX_MergeTags(const char * tags, char ** ppNewTags)
{
	if (tags == nullptr)
		return false;

	const string strTags(tags);
	string normalizedTags;
	bool ret = CRMXMgr::GetInstance().MergeTags(strTags, normalizedTags);
	if (!normalizedTags.empty())
	{
		*ppNewTags = new char[normalizedTags.length() + 1];
		strcpy(*ppNewTags, normalizedTags.c_str());
	}

	return ret;
}

LIBRMXAPI bool RMX_RPMSetViewOverlay(const RMX_VIEW_OVERLAY_PARAMS& params)
{
	std::vector<std::wstring> vecFiles;
	for (unsigned int i = 0; i < params.files.count; ++i)
	{
		vecFiles.push_back(wstring(params.files.fileArr[i]));
	}
	
	TEvalAttributeList ctxAttrs;
	for (unsigned int i = 0; i < params.ctxAttrs.count; ++i)
	{
		ctxAttrs.push_back(TEvalAttribute(wstring(params.ctxAttrs.attrs[i].key), wstring(params.ctxAttrs.attrs[i].value)));
	}
	return CRMXMgr::GetInstance().RPMSetViewOverlay(vecFiles, ctxAttrs, params.hTargetWnd,
		std::tuple<int, int, int, int>(params.displayOffset[0], params.displayOffset[1], params.displayOffset[2], params.displayOffset[3]));
}

LIBRMXAPI bool RMX_RPMClearViewOverlay(void * hTargetWnd)
{
	return CRMXMgr::GetInstance().RPMClearViewOverlay(hTargetWnd);
}

LIBRMXAPI bool RMX_GetRights(const wchar_t * nxlFilePath, unsigned long & rights)
{
	if (nxlFilePath == nullptr)
		return false;

	CRMXNxlFile nxlFile(nxlFilePath);
	std::vector<RMXFileRight> allRights;
	if (nxlFile.GetRights(allRights))
	{
		for (auto r : allRights)
			rights |= r;

		return true;
	}

	return false;
}

LIBRMXAPI bool RMX_RPMAddTrustedApp(const wchar_t * appPath)
{
	if (appPath == nullptr)
		return false;

	const wstring wstrAppPath(appPath);
	return CRMXMgr::GetInstance().RPMAddTrustedApp(wstrAppPath);
}

LIBRMXAPI bool RMX_RPMRemoveTrustedApp(const wchar_t * appPath)
{
	if (appPath == nullptr)
		return false;

	const wstring wstrAppPath(appPath);
	return CRMXMgr::GetInstance().RPMRemoveTrustedApp(wstrAppPath);
}

LIBRMXAPI bool RMX_ReProtectFile(const wchar_t * srcNxlFilePath)
{
	const wstring wstrOrigFilePath(srcNxlFilePath);
	return CRMXMgr::GetInstance().ReprotectFile(wstrOrigFilePath);
}

LIBRMXAPI bool RMX_RPMEditSaveFile(const wchar_t * srcFilePath, const wchar_t * origNxlFilePath, bool deleteSource, RMXEditSaveMode mode)
{
	const wstring wstrSrcFilePath(srcFilePath);
	wstring wstrOrigNxlFilePath;
	if (origNxlFilePath != nullptr)
		wstrOrigNxlFilePath = origNxlFilePath;
	return CRMXMgr::GetInstance().RPMEditSaveFile(wstrSrcFilePath, wstrOrigNxlFilePath, deleteSource, mode);
}

LIBRMXAPI bool RMX_AddActivityLog(const wchar_t * nxlFilePath, RMXActivityLogOperation op, RMXActivityLogResult logResult)
{
	if (nxlFilePath == nullptr)
		return false;

	CRMXNxlFile nxlFile(nxlFilePath);
	return nxlFile.AddActivityLog(op, logResult);
}

LIBRMXAPI bool RMX_EnsureNxlExtension(const wchar_t * filePath)
{
	if (filePath == nullptr)
		return false;
	CRMXNxlFile nxlFile(filePath);
	return nxlFile.EnsureNxlExtension();
}

LIBRMXAPI bool RMX_AddFileAttributes(const wchar_t * nxlFilePath, unsigned long attrs)
{
	if (nxlFilePath == nullptr)
		return false;
	CRMXNxlFile nxlFile(nxlFilePath);
	return nxlFile.AddAttributes(attrs);
}

LIBRMXAPI bool RMX_SetFileAttributes(const wchar_t * nxlFilePath, unsigned long attrs)
{
	if (nxlFilePath == nullptr)
		return false;
	CRMXNxlFile nxlFile(nxlFilePath);
	return nxlFile.SetAttributes(attrs);
}

LIBRMXAPI bool RMX_ShowFileInfoDialog(const wchar_t* nxlFilePath, const wchar_t* fileDisplyName, const wchar_t* dlgTitle)
{
	if (nxlFilePath == nullptr)
		return false;
	RPMCommonDialogWrapper dlgWrapper;
	return dlgWrapper.ShowFileInfoDialog(nxlFilePath, fileDisplyName, dlgTitle);
}

LIBRMXAPI bool RMX_ShowProtectDialog(const wchar_t* filePath, const wchar_t* buttonLabel, wchar_t** ppTags)
{
	if (filePath == nullptr)
		return false;

	RPMCommonDialogWrapper dlgWrapper;
	std::wstring retTags;
	if (dlgWrapper.ShowProtectDialog(filePath, buttonLabel, retTags) && !retTags.empty())
	{
		*ppTags = new wchar_t[retTags.length() + 1];
		wcscpy(*ppTags, retTags.c_str());

		return true;
	}

	return false;
}
