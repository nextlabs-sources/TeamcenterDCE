#include "RMXMgr.h"
// System
#include <windows.h>
#include <shlwapi.h>

// Utils
#include "..\common\CreoRMXLogger.h"
#include <RMXUtils.h>
#include <PathEx.h>
#include <SysErrorMessage.h>
// SkyDRM
#include <SDLInstance.h>
#include <SDLTypeDef.h>

#include "RMXInstance.h"
#include "RMXNxlFile.h"
#include "RMXResult.h"
#include <RMXTagHlp.h>
#include <RMXViewOverlay.h>

using namespace std;

CRMXMgr::CRMXMgr()
{
}

CRMXMgr::~CRMXMgr()
{
}

bool CRMXMgr::ProtectFile(const string& tags, const wstring& filePath, const wstring& newNxlFilePath)
{
	auto pDRmUser = CRMXInstance::GetInstance().GetDRmUser();
	if (pDRmUser == nullptr)
		return false;

	CRMXLastResult result;
	wstring newCreatedFilePath;
	vector<SDRmFileRight> rights;
	SDR_Expiration expire;
	SDR_WATERMARK_INFO watermarkinfo;
	expire.type = IExpiryType::NEVEREXPIRE;
	const string& memberId = CRMXInstance::GetInstance().GetSystemProjectMemberId();

	// Merge tags if necessary
	string normalizedTags;
	MergeTags(tags, normalizedTags);

	// Specify the folder of target file for ProtectFile API to avoid file copy,
	// due to fix the known issue in RPM: 
	// If user profile folder(e.g.:c:\users\tcadmin) or RPM secret folder is accidently set as rpm folder,
	// copy nxl file from rpm folder by trusted process will copy plain file.
	CRMXNxlFile destNxlFile(newNxlFilePath);
	newCreatedFilePath = destNxlFile.GetDirectory();
	if (!CPathEx::IsDir(newCreatedFilePath))
	{
		int ret = CPathEx::CreateDirectories(newCreatedFilePath);
		if (ret != ERROR_SUCCESS)
		{
			result = CSysErrorMessage(ret);
			LOG_ERROR_FMT(L"Failed to create directory: '%s' (error: %s)", newCreatedFilePath.c_str(), result.ToString().c_str());
			return false;
		}
	}

	/*bool readOnlyAttr = false;
	DWORD attrs = ::GetFileAttributes(filePath.c_str());
	if (attrs != INVALID_FILE_ATTRIBUTES && HAS_BIT(attrs, FILE_ATTRIBUTE_READONLY))
		readOnlyAttr = true;*/

	result = pDRmUser->ProtectFile(filePath, newCreatedFilePath, rights, watermarkinfo, expire, normalizedTags, memberId);
	CHK_ERROR_RETURN(!result, false, L"[RPM]ProtectFile failed: '%s' (error: %s)", filePath.c_str(), result.ToString().c_str());

	// Delete the unprotected source file
	CRMXNxlFile srcNxlFile(filePath);
	srcNxlFile.Delete();

	// Rename the newly created nxl file to the target file name
	// Note: RPM API always adds timestamp into new nxl file so that the new file name is different from original source file.
	CRMXNxlFile newNxlFile(newCreatedFilePath);
	if (newNxlFile.MoveTo(newNxlFilePath))
	{
		// TBD:  Needed this activity log?
		/*CRMXNxlFile nxlFile(destNxlFile.GetNxlFilePath());
		nxlFile.AddActivityLog(RMX_OProtect, RMX_RAllowed);*/
		//if (readOnlyAttr)
		//{
		//	// In ipem save as dialog, choose "remain read-only copies" to make local unprotected file read-only. 
		//	// After it gets protected, should keep this read-only file attribute.
		//	attrs = ::GetFileAttributes(filePath.c_str());
		//	attrs |= FILE_ATTRIBUTE_READONLY;
		//	SetFileAttributes(destNxlFile.GetNxlFilePath().c_str(), attrs);
		//}
			
		LOG_INFO_FMT(L"ProtectFile (tags: %s): '%s'", RMXUtils::s2ws(normalizedTags).c_str(), destNxlFile.GetNxlFilePath().c_str());
		return true;
	}

	return false;
}

bool CRMXMgr::IsRPMFolder(const std::wstring & dirPath, RMX_SAFEDIRRELATION* dirRelation /*= nullptr*/)
{
	if (PathIsNetworkPath(dirPath.c_str()))
	{
		LOG_DEBUG(L"IsRPMFolder: Network path not supported" << dirPath.c_str());
		return false;
	}

	auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmcInstance == nullptr)
		return false;

	CRMXLastResult result;

	uint32_t dirStatus;
	SDRmRPMFolderOption option;
	std::wstring fileTags;
	result = pDRmcInstance->IsRPMFolder(dirPath, &dirStatus, &option, fileTags);
	CHK_ERROR_RETURN(!result, false, L"[RPM]IsRPMFolder failed: '%s' (error: %s)", dirPath.c_str(), result.ToString().c_str());
	
	bool isRPMDir = RPMHlp::IsRPMDir(dirStatus);
	LOG_INFO_FMT(L"IsRPMFolder('%s'): %s (Relation: %d)", dirPath.c_str(), isRPMDir? L"Yes" : L"No", dirStatus);
	if (dirRelation)
	{
		*dirRelation = RPMHlp::GetRMXSafeDirType(dirStatus);
		return true;
	}
	
	// The nxl file extension is hidden in safe dir or posterity of safe dir
	return isRPMDir;
}

bool CRMXMgr::AddRPMDir(const wstring& dirPath, uint32_t option /*= RMX_RPMFOLDER_OVERWRITE | RMX_RPMFOLDER_API*/)
{
	if (PathIsNetworkPath(dirPath.c_str()))
	{
		LOG_DEBUG(L"AddRPMDir failed. Network path not supported" << dirPath.c_str());
		return false;
	}
	RMX_SAFEDIRRELATION dirRelation;
	if (IsRPMFolder(dirPath, &dirRelation) && dirRelation == RMX_IsSafeDir)
		return true; // Ignore error in case it's already rpm safe dir

	auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmcInstance == nullptr)
		return false;

	CRMXLastResult result;
	result = pDRmcInstance->AddRPMDir(dirPath, option);
	CHK_ERROR_RETURN(!result, false, L"[RPM]AddRPMDir failed: '%s '(error: %s)", dirPath.c_str(), result.ToString().c_str());

	// Workaround: Refresh new rpm folder 
	WIN32_FIND_DATA findData;
	wstring searchDir(dirPath + L"\\*.nxl");
	HANDLE hFind = FindFirstFile(searchDir.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			
		} while (FindNextFile(hFind, &findData));

		FindClose(hFind);
	}

	LOG_INFO(L"RPM dir added: " << dirPath.c_str());
	return true;
}

bool CRMXMgr::RemoveRPMDir(const std::wstring& dirPath)
{
	if (PathIsNetworkPath(dirPath.c_str()))
	{
		LOG_DEBUG(L"RemoveRPMDir failed. Network path not supported" << dirPath.c_str());
		return false;
	}
	RMX_SAFEDIRRELATION dirRelation;
	if (IsRPMFolder(dirPath, &dirRelation) && dirRelation != RMX_IsSafeDir)
		return true; // Ignore error in case it's not rpm safe dir

	auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmcInstance == nullptr)
		return false;

	CRMXLastResult result;
	result = pDRmcInstance->RemoveRPMDir(dirPath, true);
	CHK_ERROR_RETURN(!result, false, L"[RPM]RemoveRPMDir failed: '%s' (error: %s)", dirPath.c_str(), result.ToString().c_str());

	LOG_INFO(L"RPM dir removed: " << dirPath.c_str());
	return true;
}

bool CRMXMgr::RPMRegisterApp(const std::wstring& appPath)
{
	auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmcInstance == nullptr)
		return false;
	
	bool alreadyRegistered;
	if (pDRmcInstance->RPMIsAppRegistered(appPath, alreadyRegistered) && alreadyRegistered)
	{
		LOG_INFO(L"RMP app already registered: " << appPath);
		return true;
	}
		
	CRMXLastResult result;
	result = pDRmcInstance->RPMRegisterApp(appPath, LOGIN_PASSCODE);
	CHK_ERROR_RETURN(!result, false, L"[RPM]RPMRegisterApp failed: '%s' (error: %s)", appPath.c_str(), result.ToString().c_str());

	LOG_INFO(L"RMP app registered: " << appPath);
	return true;
}

bool CRMXMgr::RPMUnregisterApp(const std::wstring & appPath)
{
	auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmcInstance == nullptr)
		return false;
	
	CRMXLastResult result;
	result = pDRmcInstance->RPMUnregisterApp(appPath, LOGIN_PASSCODE);
	CHK_ERROR_RETURN(!result, false, L"[RPM]RPMUnregisterApp failed: '%s' (error: %s)", appPath.c_str(), result.ToString().c_str());

	LOG_INFO(L"RMP app unregistered: " << appPath);
	return true;
}

bool CRMXMgr::RPMAddTrustedProcess(unsigned long processId)
{
	auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmcInstance == nullptr)
		return false;

	CRMXLastResult result;
	result = pDRmcInstance->RPMAddTrustedProcess(processId, LOGIN_PASSCODE);
	CHK_ERROR_RETURN(!result, false, L"[RPM]RPMAddTrustedProcess failed: %lu (error: %s)", processId, result.ToString().c_str());

	LOG_INFO(L"Trusted process added: " << processId);
	return true;
}

bool CRMXMgr::MergeTags(const std::string& tags, std::string& normalizedTags)
{
	auto pDRmUser = CRMXInstance::GetInstance().GetDRmUser();
	if (pDRmUser == nullptr)
		return false;

	RMXLib::MergeTags(pDRmUser, tags, normalizedTags);
	return true;
}

bool CRMXMgr::RPMSetViewOverlay(std::vector<std::wstring>& vecFiles, const TEvalAttributeList& contextAttrs, void* hTargetWnd,
	const std::tuple<int, int, int, int>& displayOffset)
{
	auto pDRmInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmInstance == nullptr)
		return false;

	return RMXLib::SetViewOverlay(pDRmInstance, hTargetWnd, vecFiles, contextAttrs, displayOffset);
}

bool CRMXMgr::RPMClearViewOverlay(void * hTargetWnd)
{
	auto pDRmInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmInstance == nullptr)
		return false;

	return RMXLib::ClearViewOverlay(pDRmInstance, hTargetWnd);
}

bool CRMXMgr::RPMAddTrustedApp(const std::wstring & appPath)
{
	CHK_ERROR_RETURN(appPath.empty(), false, L"Failed to add trusted app controlled by RMX (error: %s)", L"appPath not specified");

	auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmcInstance == nullptr)
		return false;

	CRMXLastResult result;
	result = pDRmcInstance->RPMAddTrustedApp(appPath, LOGIN_PASSCODE);
	CHK_ERROR_RETURN(!result, false, L"[RPM]RPMAddTrustedApp failed: '%s' (error: %s)", appPath.c_str(), result.ToString().c_str());

	LOG_INFO(L"RMP trusted app added: " << appPath);
	return true;
}

bool CRMXMgr::RPMRemoveTrustedApp(const std::wstring & appPath)
{
	CHK_ERROR_RETURN(appPath.empty(), false, L"Failed to remove trusted app controlled by RMX (error: %s)", L"appPath not specified");

	auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmcInstance == nullptr)
		return false;

	CRMXLastResult result;
	result = pDRmcInstance->RPMRemoveTrustedApp(appPath, LOGIN_PASSCODE);
	CHK_ERROR_RETURN(!result, false, L"[RPM]RPMRemoveTrustedApp failed: '%s' (error: %s)", appPath.c_str(), result.ToString().c_str());

	LOG_INFO(L"RMP trusted app removed: " << appPath);
	return true;
}

/*
Note: ReProtectSystemBucketFile does not support the file in rpm folder, call RPMEditSaveFile instead
*/
bool CRMXMgr::ReprotectFile(const std::wstring & originalNxlFilePath)
{
	CHK_ERROR_RETURN(originalNxlFilePath.empty(), false, L"Failed to reprotect file (error: %s)", L"originalNxlFilePath not specified");

	auto pDRmUser = CRMXInstance::GetInstance().GetDRmUser();
	CHK_ERROR_RETURN(pDRmUser == nullptr, false, L"Failed to reprotect file: '%s' (error: Invalid pDRmUser)", originalNxlFilePath.c_str());

	CRMXLastResult result;
	CRMXNxlFile srcNxlFile(originalNxlFilePath);
	result = pDRmUser->ReProtectSystemBucketFile(srcNxlFile.GetNxlFilePath());
	CHK_ERROR_RETURN(!result, false, L"[RPM]ReProtectSystemBucketFile failed: '%s' (error: %s)", originalNxlFilePath.c_str(), result.ToString().c_str());

	LOG_INFO_FMT(L"ReprotectFile: '%s'", originalNxlFilePath.c_str());

	return true;
}

bool CRMXMgr::RPMEditSaveFile(const std::wstring & srcFilePath, const std::wstring & originalNXLFilePath /*= L""*/, bool deleteSource /*= false*/, RMXEditSaveMode mode /*= RMX_EditSaveMode_None*/)
{
	CHK_ERROR_RETURN(srcFilePath.empty(), false, L"Failed to RPMEditSaveFile (error: %s)", L"srcFilepath not specified");
	
	auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	CHK_ERROR_RETURN(pDRmcInstance == nullptr, false, L"Failed to RPMEditSaveFile: '%s' (error: Invalid pDRmcInstance)", srcFilePath.c_str());

	CRMXLastResult result;
	CRMXNxlFile srcNxlFile(srcFilePath);
	result = pDRmcInstance->RPMEditSaveFile(srcNxlFile.GetPlainFilePath(), originalNXLFilePath, deleteSource, (uint32_t)mode);
	CHK_ERROR_RETURN(!result, false, L"[RPM]RPMEditSaveFile failed: srcFile: '%s', originalNxlFile: '%s' (error: %s)", srcNxlFile.GetPlainFilePath().c_str(), originalNXLFilePath.c_str(), result.ToString().c_str());

	LOG_INFO_FMT(L"RPMEditSaveFile succeeded(srcFile: '%s', origFile: '%s', deleteSource: %d, mode: %d)", srcFilePath.c_str(), originalNXLFilePath.c_str(), deleteSource, (int)mode);
	return true;
}
