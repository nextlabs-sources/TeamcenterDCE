#include "RMXLInstanceImpl.h"

#include <map>
#include <vector>
#include <winerror.h>

#include "InternalTypeDef.h"
#include <PathEx.h>
#include "RMXLLogger.h"
#include "RMXLResultEx.h"
#include "RMXNxlFile.h"
#include <RMXUtils.h>
#include <RMXViewOverlay.h>
#include <SysErrorMessage.h>

#include <SDLAPI.h>
#include <SDLInstance.h>

#include "RMXLUserImpl.h"

namespace RMXLib
{
	static std::map<RMXFileRight, std::wstring> RIGHTNAMEMAP = {
		{ RMX_RIGHT_VIEW, L"RMX_RIGHT_VIEW" },
		{ RMX_RIGHT_EDIT, L"RMX_RIGHT_EDIT" },
		{ RMX_RIGHT_PRINT, L"RMX_RIGHT_PRINT" },
		{ RMX_RIGHT_CLIPBOARD, L"RMX_RIGHT_CLIPBOARD" },
		{ RMX_RIGHT_SAVEAS, L"RMX_RIGHT_SAVEAS" },
		{ RMX_RIGHT_DECRYPT, L"RMX_RIGHT_DECRYPT" },
		{ RMX_RIGHT_SCREENCAPTURE, L"RMX_RIGHT_SCREENCAPTURE" },
		{ RMX_RIGHT_SEND, L"RMX_RIGHT_SEND" },
		{ RMX_RIGHT_CLASSIFY, L"RMX_RIGHT_CLASSIFY" },
		{ RMX_RIGHT_SHARE, L"RMX_RIGHT_SHARE" },
		{ RMX_RIGHT_DOWNLOAD, L"RMX_RIGHT_DOWNLOAD" },
		{ RMX_RIGHT_WATERMARK, L"RMX_RIGHT_WATERMARK" }
	};

	CRMXLInstanceImpl::CRMXLInstanceImpl()
		: m_pSDRmcInst(nullptr), m_pRMXUser(nullptr)
	{
	}

	CRMXLInstanceImpl::CRMXLInstanceImpl(ISDRmcInstance * pSDRmcInst, ISDRmUser* pSDRmUser)
		: m_pSDRmcInst(pSDRmcInst)
	{
		m_pRMXUser = RMXL_PTRAS(new CRMXLUserImpl(pSDRmUser), IRMXUser);
	}


	CRMXLInstanceImpl::~CRMXLInstanceImpl()
	{
		if (m_pRMXUser != nullptr)
			delete m_pRMXUser;
		if (m_pSDRmcInst != nullptr)
			SDWLibDeleteRmcInstance(m_pSDRmcInst);
	}

	void CRMXLInstanceImpl::NotifyRPM(const std::wstring & path, bool isFile)
	{
		WIN32_FIND_DATA findData;
		CPathEx searchPath(path.c_str());
		searchPath /= L"*";
		if (!isFile)
		{
			// In case the specified path is folder path.
			searchPath += L".nxl";
		}
		HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{

			} while (FindNextFile(hFind, &findData));

			FindClose(hFind);
		}
	}

	bool CRMXLInstanceImpl::IsRPMDir(uint32_t sdrDirStatus)
	{
		return (sdrDirStatus & RPM_SAFEDIRRELATION_SAFE_DIR || sdrDirStatus & RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR);
	}

	RMX_RPMFolderRelation CRMXLInstanceImpl::GetRPMDirRelation(uint32_t sdrDirStatus)
	{
		RMX_RPMFolderRelation relation;
		if (sdrDirStatus & RPM_SAFEDIRRELATION_SAFE_DIR)
			relation = RMX_RPMFolder;
		else if (sdrDirStatus & RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR)
			relation = RMX_RPMInheritedFolder;
		else if (sdrDirStatus & RPM_SAFEDIRRELATION_ANCESTOR_OF_SAFE_DIR)
			relation = RMX_RPMAncestralFolder;
		else
			relation = RMX_NonRPMFolder;
		return relation;
	}

	RMXResult CRMXLInstanceImpl::IsInitFinished(bool & finished)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		result = m_pSDRmcInst->IsInitFinished(finished);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]IsInitFinished failed: '%s '", L"");

		return result;
	}

	RMXResult CRMXLInstanceImpl::GetLoginUser(IRMXUser *& pUser)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));
		if (m_pRMXUser)
			pUser = m_pRMXUser;

		return RMXResult();
	}

	RMXResult CRMXLInstanceImpl::AddRPMDir(const std::wstring & dirPath, uint32_t option /*= (RMXRPMFolderOption::RMX_RPMFOLDER_NORMAL | RMXRPMFolderOption::RMX_RPMFOLDER_API)*/)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		//RMX_SAFEDIRRELATION dirRelation;
		//if (IsRPMFolder(dirPath, &dirRelation) && dirRelation == RMX_IsSafeDir)
		//	return true; // Ignore error in case it's already rpm safe dir
		CRMXLResultEx result;
		result = m_pSDRmcInst->AddRPMDir(dirPath, option);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]AddRPMDir failed: '%s '", dirPath.c_str());

		// Workaround: Refresh new rpm folder 
		NotifyRPM(dirPath, false);

		LOG_INFO(L"AddRPMDir succeeded: " << dirPath);

		return result;
	}

	RMXResult CRMXLInstanceImpl::RemoveRPMDir(const std::wstring & dirPath, bool force /*= true*/)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		result = m_pSDRmcInst->RemoveRPMDir(dirPath, force);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RemoveRPMDir failed: '%s'", dirPath.c_str());

		LOG_INFO(L"RemoveRPMDir succeeded: " << dirPath);

		return result;
	}

	RMXResult CRMXLInstanceImpl::IsRPMFolder(const std::wstring & dirPath, bool& isRPMDir, RMX_RPMFolderRelation * relation /*= nullptr*/)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		uint32_t dirStatus = 0;
		result = m_pSDRmcInst->IsRPMFolder(dirPath, &dirStatus);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]IsRPMFolder failed: '%s'", dirPath.c_str());

		isRPMDir = IsRPMDir(dirStatus);
		LOG_INFO_FMT(L"IsRPMFolder(%s): isRPMDir=%d, relation=%d", dirPath.c_str(), isRPMDir, dirStatus);
		if (relation)
		{
			*relation = GetRPMDirRelation(dirStatus);
		}

		return result;
	}

	RMXResult CRMXLInstanceImpl::RegisterApp(const std::wstring & appPath)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		// Skip error if app is already registered	
		bool alreadyRegistered;
		if (m_pSDRmcInst->RPMIsAppRegistered(appPath, alreadyRegistered) && alreadyRegistered)
		{
			LOG_INFO(L"RMP app already registered: " << appPath);
			return RMXResult();
		}

		CRMXLResultEx result;
		result = m_pSDRmcInst->RPMRegisterApp(appPath, RMXL_LOGIN_PASSCODE);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMRegisterApp failed: '%s'", appPath.c_str());

		LOG_INFO(L"RegisterApp succeeded: " << appPath);

		return result;
	}

	RMXResult CRMXLInstanceImpl::UnregisterApp(const std::wstring & appPath)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		result = m_pSDRmcInst->RPMUnregisterApp(appPath, RMXL_LOGIN_PASSCODE);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMUnregisterApp failed: '%s'", appPath.c_str());

		LOG_INFO(L"UnregisterApp succeeded: " << appPath);

		return result;
	}

	RMXResult CRMXLInstanceImpl::IsAppRegistered(const std::wstring & appPath, bool& registered)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		result = m_pSDRmcInst->RPMIsAppRegistered(appPath, registered);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMUnregisterApp failed: '%s'", appPath.c_str());

		LOG_INFO_FMT(L"IsAppRegistered succeeded(%s): %d", appPath.c_str(), registered);

		return result;
	}

	RMXResult CRMXLInstanceImpl::NotifyRMXStatus(bool running)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		result = m_pSDRmcInst->RPMNotifyRMXStatus(running, RMXL_LOGIN_PASSCODE);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMNotifyRMXStatus failed%s", L"");

		LOG_INFO_FMT(L"RPMNotifyRMXStatus succeeded: %d", running);

		return result;
	}

	RMXResult CRMXLInstanceImpl::AddTrustedApp(const std::wstring & appPath)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		result = m_pSDRmcInst->RPMAddTrustedApp(appPath, RMXL_LOGIN_PASSCODE);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMAddTrustedApp failed: '%s'", appPath.c_str());

		LOG_INFO(L"AddTrustedApp succeeded: " << appPath);

		return result;
	}

	RMXResult CRMXLInstanceImpl::RemoveTrustedApp(const std::wstring & appPath)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));


		CRMXLResultEx result;
		result = m_pSDRmcInst->RPMRemoveTrustedApp(appPath, RMXL_LOGIN_PASSCODE);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMRemoveTrustedApp failed: '%s'", appPath.c_str());

		LOG_INFO(L"RemoveTrustedApp succeeded: " << appPath);

		return result;
	}

	RMXResult CRMXLInstanceImpl::EditCopyFile(const std::wstring & nxlFilePath, std::wstring & destPath)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		std::wstring copyFile(destPath);
		result = m_pSDRmcInst->RPMEditCopyFile(nxlFilePath, copyFile);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMEditCopyFile failed: nxlFilePath=%s, destPath=%s", nxlFilePath.c_str(), destPath.c_str());

		destPath = copyFile;
		LOG_INFO_FMT(L"EditCopyFile succeeded: nxlFilePath=%s, destPath=%s", nxlFilePath.c_str(), destPath.c_str());

		return result;
	}

	RMXResult CRMXLInstanceImpl::EditSaveFile(
		const std::wstring & filePath,
		const std::wstring & originalNXLfilePath /*= L""*/,
		bool deleteSource /*= false*/,
		uint32_t exitEdit /*= 0*/)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		result = m_pSDRmcInst->RPMEditSaveFile(filePath, originalNXLfilePath, deleteSource, exitEdit);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMEditSaveFile failed: filePath=%s, originalNXLfilePath=%s", filePath.c_str(), originalNXLfilePath.c_str());

		LOG_INFO_FMT(L"EditSaveFile succeeded: filePath=%s, originalNXLfilePath=%s", filePath.c_str(), originalNXLfilePath.c_str());

		return result;
	}

	RMXResult CRMXLInstanceImpl::RPMCopyFile(const std::wstring & existingNxlFilePath, const std::wstring & newNxlFilePath, bool deleteSource)
	{
		RMXL_LOG_ENTER;
		RMXL_ERROR_RETURN(newNxlFilePath.empty(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"New NXL file path not specified"));
		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		result = m_pSDRmcInst->RPMCopyFile(existingNxlFilePath, newNxlFilePath, deleteSource);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMCopyFile failed: existingNxlFilePath=%s, newNxlFilePath=%s", existingNxlFilePath.c_str(), newNxlFilePath.c_str());

		LOG_INFO_FMT(L"RPMCopyFile succeeded: srcNxlFilePath=%s, destNxlFilePath=%s", existingNxlFilePath.c_str(), newNxlFilePath.c_str());

		return result;
	}

	// Since RPM10.6.1911, native RPMCopyFIle API is provided. Comment out below RMX impl.
	//RMXResult CRMXLInstanceImpl::RPMCopyFile(const std::wstring & existingNxlFilePath, const std::wstring & newNxlFilePath, bool deleteSource)
	//{
	//	RMXL_LOG_ENTER;

	//	RMXL_ERROR_RETURN(newNxlFilePath.empty(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"New NXL file path not specified"));
	//	RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

	//	CRMXLResultEx result;
	//	// Ignore if target file path is same to the source file.
	//	if (_wcsicmp(existingNxlFilePath.c_str(), newNxlFilePath.c_str()) == 0)
	//		return result;
	//
	//	// In case the source file not exist
	//	CRMXNxlFile existingFile(existingNxlFilePath);
	//	RMXL_ERROR_RETURN_FMT(!existingFile.Exist(), RMX_RESULT2(ERROR_FILE_NOT_FOUND, L"NXL file to be copied not found"), L"RPMCopyFile failed: %s", existingNxlFilePath.c_str());
	//	
	//	// In case the target path is not file path.
	//	CRMXNxlFile newFile(newNxlFilePath);
	//	RMXL_ERROR_RETURN_FMT(newFile.GetNativeFileName().empty(), RMX_RESULT2(ERROR_FILE_INVALID, L"Invalid target file path specified"), L"RPMCopyFile failed: %s", newNxlFilePath.c_str());

	//	// In case the source folder is not rpm folder
	//	bool isSourceRPMDir = false;
	//	if (!IsRPMFolder(existingFile.GetDirectory(), isSourceRPMDir) || !isSourceRPMDir)
	//	{
	//		RMXL_ERROR_RETURN_FMT(true, RMX_RESULT2(ERROR_INTERNAL_ERROR, L"NXL file to be copied not in rpm folder"), L"RPMCopyFile failed: %s", existingNxlFilePath.c_str());
	//	}

	//	// Delete target file if it is existing
	//	bool isTargetRPMDir = false;
	//	if (IsRPMFolder(newFile.GetDirectory(), isTargetRPMDir) && isTargetRPMDir)
	//	{
	//		// In RPM Folder, ensure to delete both nxl file and non nxl file
	//		if (newFile.Exist())
	//		{
	//			auto innerRet = RPMDeleteFile(newFile.GetNativeFilePath());
	//			if (!innerRet)
	//				return innerRet;
	//		}
	//		else if (newFile.NativeFileExist() && !CPathEx::RemoveFile(newFile.GetNativeFilePath().c_str()))
	//		{
	//			DWORD errorCode = (errno == EACCES) ? ERROR_ACCESS_DENIED : ERROR_FILE_NOT_FOUND;
	//			result = RMX_RESULT2(errorCode, (LPCTSTR)CSysErrorMessage(errorCode));
	//			LOG_ERROR_FMT(L"RPMCopyFile failed due to target NXL file exists but failed to overwrite: %s (error: %s)", newNxlFilePath.c_str(), result.ToString().c_str());
	//			return result;
	//		}
	//	}
	//	// In Non rpm folder, nxl file and it's unprotected file can co-exist. 
	//	// Only delete nxl file. Let caller decide if unprotected file needs to be deleted.
	//	else if (newFile.Exist() && !CPathEx::RemoveFile(newFile.GetNxlFilePath().c_str()))
	//	{
	//		DWORD errorCode = (errno == EACCES) ? ERROR_ACCESS_DENIED : ERROR_FILE_NOT_FOUND;
	//		result = RMX_RESULT2(errorCode, (LPCTSTR)CSysErrorMessage(errorCode));
	//		LOG_ERROR_FMT(L"RPMCopyFile failed due to target NXL file exists but failed to overwrite: %s (error: %s)", newNxlFilePath.c_str(), result.ToString().c_str());
	//		return result;
	//	}

	//	// Start an untrusted process to copy nxl file in RPM folder.
	//	// Note: Trusted process will copy the plain file which is unexpected copying behavior.
	//	std::wstring cmdExe = RMXUtils::getEnv(L"COMSPEC");
	//	RMXL_ERROR_RETURN(cmdExe.empty(), RMX_RESULT2(ERROR_INVALID_PARAMETER, L"'COMSPEC' environment variable undefined"));
	//	
	//	std::wstring cmdArgs;
	//	cmdArgs += cmdExe + L" /c copy \"" + existingFile.GetNxlFilePath() + L"\" \"" + newFile.GetNxlFilePath() + L"\"";
	//	LOG_DEBUG_FMT(L"Start to copy NXL file: '%s'...", cmdArgs.c_str());

	//	// Create the child process.  
	//	LPWSTR szCmd = const_cast<wchar_t*>(cmdArgs.c_str());
	//	STARTUPINFO si;
	//	ZeroMemory(&si, sizeof(STARTUPINFO));
	//	si.cb = sizeof(STARTUPINFO);
	//	si.dwFlags = STARTF_USESHOWWINDOW;
	//	si.wShowWindow = SW_HIDE;
	//	PROCESS_INFORMATION pi;
	//	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	//	bool bSuccess = false;
	//	if (CreateProcess(NULL, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	//	{
	//		DWORD dwWaitResult = WaitForSingleObject(pi.hProcess, INFINITE);

	//		// Close process and thread handles. 
	//		CloseHandle(pi.hThread);
	//		CloseHandle(pi.hProcess);

	//		// if the process finished, we break out
	//		if (dwWaitResult == WAIT_OBJECT_0)
	//		{
	//			bSuccess = true;
	//		}
	//		else if (dwWaitResult == WAIT_FAILED)
	//		{
	//			auto err = GetLastError();
	//			result = RMX_RESULT2(err, (LPCTSTR)CSysErrorMessage(err));
	//			LOG_ERROR_FMT(L"RPMCopyFile failed: %s (error: %)", existingNxlFilePath.c_str(), result.ToString().c_str());
	//			return result;
	//		}
	//	}

	//	if (bSuccess)
	//	{
	//		if (deleteSource)
	//		{
	//			RPMDeleteFile(existingFile.GetNativeFilePath());
	//		}
	//		if (isTargetRPMDir)
	//		{
	//			NotifyRPM(newNxlFilePath, true);
	//		}
	//		LOG_INFO_FMT(L"RPMCopyFile succeeded: srcNxlFilePath=%s, destNxlFilePath=%s", existingNxlFilePath.c_str(), newNxlFilePath.c_str());
	//	}
	//	
	//	return result;
	//}

	RMXResult CRMXLInstanceImpl::RPMDeleteFile(const std::wstring & filePath)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		result = m_pSDRmcInst->RPMDeleteFile(filePath);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMDeleteFile failed: '%s'", filePath.c_str());

		LOG_INFO_FMT(L"RPMDeleteFile succeeded: %s", filePath.c_str());

		return result;
	}

	RMXResult CRMXLInstanceImpl::GetFileRights(const std::wstring & filePath, std::vector<RMXFileRight>& rights)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		std::vector<std::pair<SDRmFileRight, std::vector<SDR_WATERMARK_INFO>>> rightsAndWatermarks;
		result = m_pSDRmcInst->RPMGetFileRights(filePath, rightsAndWatermarks, 0);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMGetFileRights failed: '%s'", filePath.c_str());

		std::wstring rNames;
		for (const auto& rw : rightsAndWatermarks)
		{
			auto rmxRight = (RMXFileRight)rw.first;
			rights.push_back(rmxRight);
			if (!rNames.empty())
				rNames += L" OR ";
			rNames += RIGHTNAMEMAP[rmxRight];
		}

		LOG_INFO_FMT(L"GetFileRights succeeded(%s): %s", filePath.c_str(), rNames.c_str());

		return result;
	}

	RMXResult CRMXLInstanceImpl::CheckFileRight(const std::wstring & filePath, RMXFileRight right, bool & allowed, bool audit /*= false*/)
	{
		RMXL_LOG_ENTER;

		CRMXLResultEx result;
		std::vector<RMXFileRight> fileRights;
		auto ret = GetFileRights(filePath, fileRights);
		if (!ret)
		{
			return ret;
		}

		static std::map<RMXFileRight, RMXActivityLogOperation> sR2LogMap = {
			{ RMX_RIGHT_VIEW, RMX_OView },
			{ RMX_RIGHT_EDIT, RMX_OEdit },
			{ RMX_RIGHT_PRINT, RMX_OPrint },
			{ RMX_RIGHT_CLIPBOARD, RMX_OCopyContent },
			{ RMX_RIGHT_SAVEAS, RMX_ODownload },
			{ RMX_RIGHT_DECRYPT, RMX_ODecrypt },
			{ RMX_RIGHT_SCREENCAPTURE, RMX_OCaptureScreen },
			{ RMX_RIGHT_SEND, RMX_OShare },
			{ RMX_RIGHT_CLASSIFY, RMX_OClassify },
			{ RMX_RIGHT_SHARE, RMX_OReshare },
			{ RMX_RIGHT_DOWNLOAD, RMX_ODownload },
			{ RMX_RIGHT_WATERMARK, RMX_OView }
		};

		allowed = false;
		for (auto r : fileRights)
		{
			// Check if the right is granted.
			if (right & r)
			{
				allowed = true;
				break;
			}
		}
	
		LOG_INFO_FMT("CheckFileRight succeeded(%s): %s, Result: %d", filePath.c_str(), RIGHTNAMEMAP[right].c_str(), allowed);

		if (audit)
			m_pRMXUser->AddActivityLog(filePath, sR2LogMap[right], allowed ? RMX_RAllowed : RMX_RDenied);

		return RMXResult();
	}

	RMXResult CRMXLInstanceImpl::GetFileStatus(const std::wstring & filePath, bool & isProtected, RMX_RPMFolderRelation * dirRelation /*= nullptr*/)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		uint32_t dirStatus = 0;
		result = m_pSDRmcInst->RPMGetFileStatus(filePath, &dirStatus, &isProtected);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMGetFileStatus failed: '%s'", filePath.c_str());

		if (dirRelation)
		{
			*dirRelation = GetRPMDirRelation(dirStatus);
		}		
		LOG_INFO_FMT(L"GetFileStatus succeeded(%s): isProtected=%d, dirRelation=%d", filePath.c_str(), isProtected, dirStatus);

		return result;
	}

	RMXResult CRMXLInstanceImpl::ReadFileTags(const std::wstring & filePath, std::string & tags)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		std::wstring outTags;
		result = m_pSDRmcInst->RPMReadFileTags(filePath, outTags);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]ReadFileTags failed: '%s'", filePath.c_str());

		// TODO: All other SkyDRM API treats tags as string, except thsi API.
		// Overall, we handle it as string/char*
		tags = RMXUtils::ws2s(outTags);
		LOG_INFO_FMT(L"ReadFileTags succeeded(%s): %s", filePath.c_str(), outTags.c_str());

		return result;
	}

	RMXResult CRMXLInstanceImpl::RPMGetFileInfo(const std::wstring & filepath, std::string & duid, std::string & tags, std::string & tokenGroup,
		std::string & creatorId, std::string & infoExt, DWORD & attributes, RMX_RPMFolderRelation & dirRelation, BOOL &isNXLFile)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));


		CRMXLResultEx result;
		std::vector<std::pair<SDRmFileRight, std::vector<SDR_WATERMARK_INFO>>> userRightsAndWatermarks;
		std::vector<SDRmFileRight> rights;
		SDR_WATERMARK_INFO waterMark;
		SDR_Expiration expiration;
		DWORD dirStatus;
		DWORD isNXL;
		result = m_pSDRmcInst->RPMGetFileInfo(filepath, duid, userRightsAndWatermarks, rights, waterMark, expiration,
			tags, tokenGroup, creatorId, infoExt, attributes, dirStatus, isNXL);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMGetFileInfo failed: nxlFilePath=%s", filepath.c_str());

		dirRelation = GetRPMDirRelation(dirStatus);
		isNXLFile = (isNXL != 0);
		LOG_INFO_FMT(L"RPMGetFileInfo succeeded(%s):", filepath.c_str());
		LOG_INFO("\t-duid:" << duid.c_str());
		LOG_INFO("\t-tags:" << tags.c_str());
		LOG_INFO("\t-tokenGroup:" << tokenGroup.c_str());
		LOG_INFO("\t-creatorId:" << creatorId.c_str());
		LOG_INFO("\t-infoExt:" << infoExt.c_str());
		LOG_INFO("\t-attributes:" << attributes);
		LOG_INFO("\t-isRPMFolder:" << dirStatus);
		LOG_INFO("\t-isNXLFile:" << isNXLFile);
		return result;
	}

	RMXResult CRMXLInstanceImpl::RPMSetFileAttributes(const std::wstring &nxlFilePath, DWORD dwFileAttributes)
	{
		RMXL_LOG_ENTER;
		
		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		CRMXLResultEx result;
		std::wstring outTags;
		result = m_pSDRmcInst->RPMSetFileAttributes(nxlFilePath, dwFileAttributes);
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMSetFileAttributes failed: nxlFilePath=%s, dwFileAttributes=%u", nxlFilePath.c_str(), dwFileAttributes);

		LOG_INFO_FMT(L"RPMSetFileAttributes succeeded(%s): %u", nxlFilePath.c_str(), dwFileAttributes);
		return result;
	}

	RMXResult CRMXLInstanceImpl::SetViewOveraly(const RMX_VIEWOVERLAY_PARAMS & params)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		TEvalAttributeList attrList;
		for (const auto& attr : params.vecCtxAttrs)
		{
			attrList.push_back(TEvalAttribute(std::wstring(attr.key), std::wstring(attr.value)));
		}
		bool succeeded = RMXLib::SetViewOverlay(m_pSDRmcInst, params.hTargetWnd, params.vecTags, attrList,
			std::tuple<int, int, int, int>(params.displayOffsets[0], params.displayOffsets[1], params.displayOffsets[2], params.displayOffsets[3]));
		RMXL_ERROR_RETURN(!succeeded, RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Failed to SetViewOverlay"));

		return RMXResult();
	}

	RMXResult CRMXLInstanceImpl::ClearViewOverlay(void * hTargetWnd)
	{
		RMXL_LOG_ENTER;

		RMXL_ERROR_RETURN(!IsInit(), RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pSDRmcInst"));

		bool succeeded = RMXLib::ClearViewOverlay(m_pSDRmcInst, hTargetWnd);
		RMXL_ERROR_RETURN(!succeeded, RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Failed to ClearViewOverlay"));

		return RMXResult();
	}

} //namespace RMXLib