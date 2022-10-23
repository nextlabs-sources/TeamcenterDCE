#include "RMXNxlFile.h"
// System
#include <map>
// Utils
#include <RMXLogInc.h>
#include <PathEx.h>
#include <RMXUtils.h>
#include <SysErrorMessage.h>
// SkyDRM
#include <SDLInstance.h>
#include <SDLTypeDef.h>

#include "RMXInstance.h"
#include "RMXResult.h"
#include "RMXTypes.h"

using namespace std;

static map<RMXFileRight, wstring> RIGHTSMAP = {
	{ RMX_RIGHT_VIEW, L"View" },
	{ RMX_RIGHT_EDIT, L"Edit" },
	{ RMX_RIGHT_PRINT, L"Print" },
	{ RMX_RIGHT_CLIPBOARD, L"Clipboard" },
	{ RMX_RIGHT_SAVEAS, L"Save As" },
	{ RMX_RIGHT_DECRYPT, L"Extract" },
	{ RMX_RIGHT_SCREENCAPTURE, L"Screen Capture" },
	{ RMX_RIGHT_SEND, L"Send" },
	{ RMX_RIGHT_CLASSIFY, L"Classify" },
	{ RMX_RIGHT_SHARE, L"Share" },
	{ RMX_RIGHT_DOWNLOAD, L"Save As" },
	{ RMX_RIGHT_WATERMARK, L"Watermark" }
};

std::wstring _GetNXLHelperExe()
{
	// Look for nxlHelper.exe in creo rmx root folder
	static const std::wstring rmxRoot = RMXUtils::getEnv(ENV_RMX_ROOT);
	if (!rmxRoot.empty())
	{
		CPathEx nxlHelper(rmxRoot.c_str());
		nxlHelper /= RMX_TOOLS_FOLDERNAME;
		nxlHelper /= EXE_NXL_HELPER;
		
		if (CPathEx::IsFile(nxlHelper.ToString()))
			return nxlHelper.ToString();
	}

	return std::wstring();
}

CRMXNxlFile::CRMXNxlFile(const std::wstring & filePath)
{
	m_origFilePath = filePath;
	m_nxlFilePath = filePath;
	if (!filePath.empty())
	{
		CPathEx origFile(filePath.c_str());
		if (_wcsnicmp(origFile.GetExtension().c_str(), NXL_FILE_EXT, origFile.GetExtension().size()) == 0)
		{
			m_plainFilePath = filePath.substr(0, wcslen(filePath.c_str()) - 4);
		}
		else
		{
			m_nxlFilePath += NXL_FILE_EXT;
			m_plainFilePath = filePath;
		}
		m_directory = origFile.GetParentPath();
	}
}

CRMXNxlFile::~CRMXNxlFile()
{
}

bool CRMXNxlFile::operator==(const std::wstring & nxlFilePath)
{
	return (_wcsnicmp(m_nxlFilePath.c_str(), nxlFilePath.c_str(), nxlFilePath.size()) == 0);
}

bool CRMXNxlFile::IsProtected() const
{
	CRMXLastResult result;
	bool isProtected = false;
	bool nxlExist = false;
	wstring filePathToCheck;
	if (IsInRPMFolder())
	{
		if (nxlExist = CPathEx::IsFile(m_nxlFilePath))
		{
			filePathToCheck = m_nxlFilePath;
		}
		else if (CPathEx::IsFile(m_plainFilePath))
		{
			filePathToCheck = m_plainFilePath;
		}
		else
		{
			result = CSysErrorMessage(ERROR_FILE_NOT_FOUND);
			CHK_ERROR_RETURN(true, false, L"Failed to check if the file is protected: '%s' (error: %s)", m_origFilePath.c_str(), result.ToString().c_str());
		}
	}
	else
	{
		if (CPathEx::IsFile(m_origFilePath))
		{
			filePathToCheck = m_origFilePath;
			nxlExist = (m_origFilePath == m_nxlFilePath);
		}
		else
		{
			result = CSysErrorMessage(ERROR_FILE_NOT_FOUND);
			CHK_ERROR_RETURN(true, false, L"Failed to check if the file is protected: '%s' (error: %s)", m_origFilePath.c_str(), result.ToString().c_str());
		}
	}

	auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmcInstance)
	{
		uint32_t dirStatus;
		result = pDRmcInstance->RPMGetFileStatus(filePathToCheck, &dirStatus, &isProtected);
		CHK_ERROR(!result, L"[RPM]RPMGetFileStatus failed: '%s' (error: %s)", filePathToCheck.c_str(), result.ToString().c_str());
	}
	
	// If any error returns from RPMGetFileStatus(user not logged in),
	// call external untrusted process to check.
	if (!result)
	{
		static const wstring nxlHelperExe = _GetNXLHelperExe();
		if (nxlHelperExe.empty())
		{
			result = CRMXResult(RMX_ERROR_NXLHELPER_NOTFOUND, L"Cannot find Nxlhelper.exe in '$ENV_RMX_ROOT\tools' folder");
			LOG_ERROR(result.ToString().c_str());
			return false;
		}

		const wstring cmd = L"\"" + nxlHelperExe + L"\" -isprotected \"" + filePathToCheck + L"\"";
		string retNxlHeper;
		if (RMXUtils::ExecuteCmd(cmd, retNxlHeper))
		{
			// Returns either nxlv1(RMC) or nxlv2(SkyDRM) if it's protected.
			if (retNxlHeper.find("nxl") == 0)
				isProtected = true;
			else if (retNxlHeper.find("nonnxl") != 0)
			{
				// Print out some error like "NotExist" if file not found
				result = CRMXResult(ERROR_FILE_NOT_FOUND, RMXUtils::s2ws(retNxlHeper));
				LOG_ERROR_FMT(L"Failed to check if the file is protected: '%s' (error: '%s')", filePathToCheck.c_str(), result.ToString().c_str());
				return false;
			}
		}
	}
	
	// Note: In order to support a special case - The file is protected without .nxl extension,
	// return true even if the file doesn't have .nxl extension(Invalid nxl file).
	// This is different behavior between IsValidNXL() and IsProtected()
	if (isProtected && !nxlExist)
	{
		result = CRMXResult(RMX_ERROR_INVALID_NXLFILE, L"Invalid nxl file. Missing .nxl extension");
		LOG_ERROR_FMT(L"Invalid protected file '%s' (error: missing .nxl extension)", filePathToCheck.c_str());
	}
	else
		LOG_INFO_FMT(L"IsProtectedFile: '%s'? %s", filePathToCheck.c_str(), isProtected? L"yes" : L"no");

	return isProtected;
}

bool CRMXNxlFile::IsValidNxl() const
{
	return CPathEx::IsFile(m_nxlFilePath) && IsProtected();
}

bool CRMXNxlFile::IsInRPMFolder() const
{
	CRMXLastResult result;
	if (!CPathEx::IsDir(m_directory))
	{
		result = CSysErrorMessage(ERROR_PATH_NOT_FOUND);
		LOG_ERROR_FMT(L"Failed to check if RPM folder '%s' (error: %s)", m_directory.c_str(), result.ToString().c_str());
		return false;
	}
	auto pDRmcInstnace = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmcInstnace)
	{
		uint32_t dirStatus;
		SDRmRPMFolderOption option;
		std::wstring fileTags;
		result = pDRmcInstnace->IsRPMFolder(m_directory, &dirStatus, &option, fileTags);
		CHK_ERROR_RETURN(!result, false, L"[RPM]IsRPMFolder failed: '%s' (error: %s)", m_directory.c_str(), result.ToString().c_str());

		bool isRPMDir = RPMHlp::IsRPMDir(dirStatus);
		LOG_INFO_FMT(L"IsFileInRPMFolder('%s') ? %s", m_origFilePath.c_str(), isRPMDir ? L"Yes" : L"No");
		return isRPMDir;
	}
	return false;
}

std::string CRMXNxlFile::GetTags()
{
	CRMXLastResult result;
	std::wstring tags(L"");
	auto pDRmInst = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmInst)
	{
		result = pDRmInst->RPMReadFileTags(m_nxlFilePath, tags);
		CHK_ERROR_RETURN(!result, std::string(""), L"[RPM]RPMReadFileTags failed: '%s' (error: '%s')", m_nxlFilePath.c_str(), result.ToString().c_str());

		LOG_INFO_FMT(L"GetTags('%s') : '%s'", m_nxlFilePath.c_str(), tags.c_str());
	}

	return RMXUtils::ws2s(tags);
}
/*
std::string CRMXNxlFile::GetTags()
{
	if (IsInRPMFolder())
	{
		// ISDRmUser::OpenFile does not support RPM dir
		// As workaround, read the tags from file directly via untrusted process.
		return ReadTagsFromFile();
	}

	CRMXLastResult result;
	std::string tags;
	auto pDRmUser = CRMXInstance::GetInstance().GetDRmUser();
	if (pDRmUser && IsValidNxl())
	{
		ISDRmNXLFile* pNxlFile = nullptr;
		result = pDRmUser->OpenFile(m_nxlFilePath.c_str(), &pNxlFile);
		CHK_ERROR(!result || pNxlFile == nullptr, L"Failed to open nxl file: '%s' (error: %s)", m_nxlFilePath.c_str(), result.ToString().c_str());

		if (result && pNxlFile)
		{
			tags = pNxlFile->GetTags();
			LOG_INFO_FMT(L"Get tags for nxl file('%s') : '%s'", m_nxlFilePath.c_str(), RMXUtils::s2ws(tags).c_str());
			pDRmUser->CloseFile(pNxlFile);
		}
	}
	return tags;
}*/

std::wstring CRMXNxlFile::GetRightName(unsigned long rights)
{
	wstring rightNames;
	for (const auto& r : RIGHTSMAP)
	{
		if (rights & r.first)
		{
			if (rightNames.empty())
				rightNames = r.second;
			else
				rightNames += L" OR " + r.second;
		}
	}
	return rightNames;
}

bool CRMXNxlFile::GetRights(std::vector<RMXFileRight>& rights) const
{
	vector<SDRmFileRight> fileRights;
	auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	CHK_ERROR_RETURN(pDRmcInstance == nullptr, false, L"Failed to get file rights: '%s' (error: Invalid DRmcInstance)", m_nxlFilePath.c_str());

	CRMXLastResult result;
	std::vector<std::pair<SDRmFileRight, std::vector<SDR_WATERMARK_INFO>>> rightsAndWatermarks;
	result = pDRmcInstance->RPMGetFileRights(m_nxlFilePath.c_str(), rightsAndWatermarks, 0);
	CHK_ERROR_RETURN(!result, false, L"[RPM]RPMGetFileRights failed: '%s' (error: %s)", m_nxlFilePath.c_str(), result.ToString().c_str());

	wstring rightNames;
	for (auto r : rightsAndWatermarks)
	{
		rights.push_back((RMXFileRight)r.first);
		if (rightNames.empty())
			rightNames = RIGHTSMAP[(RMXFileRight)r.first];
		else
			rightNames += L" OR " + RIGHTSMAP[(RMXFileRight)r.first];
	}

	LOG_INFO_FMT(L"GetRights('%s'): %s", m_nxlFilePath.c_str(), rightNames.c_str());
	return true;
}

bool CRMXNxlFile::CheckRights(unsigned long rights, bool audit)
{
	std::vector<RMXFileRight> fileRights;
	if (!GetRights(fileRights))
		return false;
	unsigned long lFileRights = 0;
	for (auto r : fileRights)
	{
		lFileRights |= r;
	}


	return HasRights(lFileRights, rights, audit);
}

bool CRMXNxlFile::HasRights(unsigned long allRights, unsigned long rightsToCheck, bool audit)
{
	// Check if single right is granted.
	auto callCheck = [&](DWORD r) -> bool {
		bool grantedRight = false;
		if (HAS_BIT(allRights, r))
		{
			grantedRight = true;
		}

		const wstring& rightNamesToCheck = GetRightName(r);
		LOG_INFO_FMT("CheckRights('%s'): %s, Result: %d ", m_nxlFilePath.c_str(), rightNamesToCheck.c_str(), grantedRight);

		if (audit)
			AddActivityLogByRight(r, grantedRight ? RMX_RAllowed : RMX_RDenied, false);


		return 	grantedRight;
	};
	// Extract multiple rights to check
	if (HAS_BIT(rightsToCheck, RMX_RIGHT_VIEW) && !callCheck(RMX_RIGHT_VIEW))
		return false;

	if (HAS_BIT(rightsToCheck, RMX_RIGHT_EDIT) && !callCheck(RMX_RIGHT_EDIT))
		return false;

	if (HAS_BIT(rightsToCheck, RMX_RIGHT_PRINT) && !callCheck(RMX_RIGHT_PRINT))
		return false;

	if (HAS_BIT(rightsToCheck, RMX_RIGHT_DOWNLOAD) && !callCheck(RMX_RIGHT_DOWNLOAD))
		return false;

	if (HAS_BIT(rightsToCheck, RMX_RIGHT_DECRYPT) && !callCheck(RMX_RIGHT_DECRYPT))
		return false;

	if (HAS_BIT(rightsToCheck, RMX_RIGHT_SAVEAS) && !callCheck(RMX_RIGHT_SAVEAS))
		return false;

	if (HAS_BIT(rightsToCheck, RMX_RIGHT_CLIPBOARD) && !callCheck(RMX_RIGHT_CLIPBOARD))
		return false;

	if (HAS_BIT(rightsToCheck, RMX_RIGHT_SCREENCAPTURE) && !callCheck(RMX_RIGHT_SCREENCAPTURE))
		return false;

	if (HAS_BIT(rightsToCheck, RMX_RIGHT_CLASSIFY) && !callCheck(RMX_RIGHT_CLASSIFY))
		return false;

	if (HAS_BIT(rightsToCheck, RMX_RIGHT_SHARE) && !callCheck(RMX_RIGHT_SHARE))
		return false;

	return true;
}

bool CRMXNxlFile::CopyTo(const std::wstring & destFilePath, bool deleteSource)
{
	if (*this == destFilePath)
		return true; //Ignore

	CRMXLastResult result;
	if (!CPathEx::IsFile(m_origFilePath))
	{
		result = CSysErrorMessage(ERROR_FILE_NOT_FOUND);
		LOG_ERROR_FMT(L"Failed to copy nxl file '%s' (error: %s)", m_origFilePath.c_str(), result.ToString().c_str());
		return false;
	}
	bool bSuccess = false;
	// CopyFile API with overwrite flag does not work well.
	// Need to manually remove old file before copying
	CRMXNxlFile destNxlFile(destFilePath);
	destNxlFile.Delete();
	const wstring& destNxlFilePath = destNxlFile.GetNxlFilePath();

	if (!CPathEx::IsDir(destNxlFile.GetDirectory()))
	{
		int ret = CPathEx::CreateDirectories(destNxlFile.GetDirectory());
		if (ret != ERROR_SUCCESS)
		{
			result = CSysErrorMessage(ret);
			LOG_ERROR_FMT(L"Failed to create directory: '%s' (error: %s)", destNxlFile.GetDirectory().c_str(), result.ToString().c_str());
			return false;
		}
	}
	if (IsInRPMFolder())
	{
		// Start an untrusted process to copy nxl file in RPM folder.
		// Note: Trusted process will copy the plain file which is unexpected copying behavior.
		wstring cmdExe = RMXUtils::getEnv(L"COMSPEC");
		if (cmdExe.empty())
		{
			result = CSysErrorMessage(ERROR_INVALID_PARAMETER);
			LOG_ERROR_STR(L"'COMSPEC' environment variable undefined");
			return false;
		}

		std::wstring cmdArgs;
		cmdArgs += cmdExe + L" /c copy \"" + m_nxlFilePath.c_str() + L"\" \"" + destNxlFilePath.c_str() + L"\"";
		LOG_DEBUG_FMT(L"Start to execute command line to copy file: '%s'...", cmdArgs.c_str());

		// Create the child process.  
		LPWSTR szCmd = const_cast<wchar_t*>(cmdArgs.c_str());
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

		if (CreateProcess(NULL, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			bSuccess = false;
			DWORD dwWaitResult = WaitForSingleObject(pi.hProcess, INFINITE);

			// Close process and thread handles. 
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);

			// if the process finished, we break out
			if (dwWaitResult == WAIT_OBJECT_0)
			{
				LOG_INFO_FMT(L"CopyNxlFile completed ('%s' to '%s')", m_nxlFilePath.c_str(), destNxlFilePath.c_str());
				bSuccess = true;
			}
			else if (dwWaitResult == WAIT_FAILED)
			{
				result = CSysErrorMessage(GetLastError());
				LOG_ERROR_FMT(L"Failed to copy file from '%s' to '%s' (error: %s)", m_nxlFilePath.c_str(), destNxlFilePath.c_str(), result.ToString().c_str());
			}
		}
	}
	else if (CopyFile(m_nxlFilePath.c_str(), destNxlFilePath.c_str(), FALSE) == 0)
	{
		result = CSysErrorMessage(GetLastError());
		LOG_ERROR_FMT(L"Failed to copy file from '%s' to '%s' (error: %s)", m_nxlFilePath.c_str(), destNxlFilePath.c_str(), result.ToString().c_str());
	}
	else
	{
		LOG_INFO_FMT(L"CopyNxlFile completed. ('%s' to '%s')", m_nxlFilePath.c_str(), destNxlFilePath.c_str());
		bSuccess = true;
	}

	if (bSuccess && destNxlFile.IsInRPMFolder())
	{
		// Call FindFirstFile to notify driver there is a new NXL file
		WIN32_FIND_DATA pNextInfo;
		HANDLE hFind = FindFirstFile(destNxlFilePath.c_str(), &pNextInfo);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);
		}
	}

	if (deleteSource)
		Delete();

	return bSuccess;
}

bool CRMXNxlFile::MoveTo(const std::wstring & destFilePath)
{
	if (*this == destFilePath)
		return true; //Ignore

	CRMXLastResult result;
	if (!CPathEx::IsFile(m_origFilePath))
	{
		result = CSysErrorMessage(ERROR_FILE_NOT_FOUND);
		LOG_ERROR_FMT(L"Failed to move nxl file '%s' (error: %s)", m_origFilePath.c_str(), result.ToString().c_str());
		return false;
	}
	bool bSuccess = false;
	
	CRMXNxlFile destNxlFile(destFilePath);
	destNxlFile.Delete();

	wstring pathToMove;
	bool isInRPMDir = destNxlFile.IsInRPMFolder();
	if (isInRPMDir)
	{
		pathToMove = destNxlFile.GetPlainFilePath();
	}
	else
	{
		pathToMove = destNxlFile.GetNxlFilePath();
	}
	if (MoveFile(m_nxlFilePath.c_str(), pathToMove.c_str()) == 0)
	{
		result = CSysErrorMessage(GetLastError());
		LOG_ERROR_FMT(L"Failed to move file from '%s' to '%s' (error: %s)", m_nxlFilePath.c_str(), pathToMove.c_str(), result.ToString().c_str());
	}
	else
	{
		LOG_INFO_FMT(L"MoveNxlFile from '%s' to '%s'", m_nxlFilePath.c_str(), pathToMove.c_str());
		bSuccess = true;
	}

	if (bSuccess)
	{
		Delete();

		if (isInRPMDir)
		{
			// Call FindFirstFile to notify driver there is a new NXL file
			WIN32_FIND_DATA pNextInfo;
			HANDLE hFind = FindFirstFile(pathToMove.c_str(), &pNextInfo);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				FindClose(hFind);
			}
		}	
	}

	return bSuccess;
}

bool CRMXNxlFile::Delete()
{
	CRMXLastResult result;
	if (CPathEx::IsFile(m_nxlFilePath.c_str()))
	{
		if (IsValidNxl() && IsInRPMFolder())
		{
			// RPMDeleteFile will delete both nxl file and its hidden plain file.
			auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
			if (pDRmcInstance)
			{
				result = pDRmcInstance->RPMDeleteFile(m_plainFilePath);
				CHK_ERROR(!result, L"[RPM]RPMDeleteFile failed: '%s' (error: %s)", m_plainFilePath.c_str(), result.ToString().c_str());
				if (result)
					LOG_INFO_FMT("DeleteNxlFile in RPM folder: '%s'", m_plainFilePath.c_str());
			}
		}
		else
		{
			if (!CPathEx::RemoveFile(m_nxlFilePath.c_str()))
			{
				DWORD errorCode = (errno == EACCES) ? ERROR_ACCESS_DENIED : ERROR_FILE_NOT_FOUND;	
				result = CSysErrorMessage(errorCode);
				CHK_ERROR_RETURN(true, false, L"Cannot delete file: '%s' (error: %s)", m_nxlFilePath.c_str(), result.ToString().c_str());			
			}
			else
			{
				LOG_INFO_FMT("DeleteNxlFile: '%s'", m_nxlFilePath.c_str());
			}
		}
	}

	// In non RPM folder, nxl file and its plain file may co-exist in same folder,
	// need to delete the plain file explicitly 
	// Note: This is special logic for CAD RMX only: Don't allow plain file to be available once it's protected. 
	// So always delete it here.
	if (CPathEx::IsFile(m_plainFilePath.c_str()))
	{
		if (!CPathEx::RemoveFile(m_plainFilePath.c_str()))
		{
			DWORD errorCode = (errno == EACCES) ? ERROR_ACCESS_DENIED : ERROR_FILE_NOT_FOUND;
			result = CSysErrorMessage(errorCode);
			CHK_ERROR_RETURN(true, false, L"Cannot delete file: '%s' (error: %s)", m_plainFilePath.c_str(), result.ToString().c_str());
		}
		else
		{
			LOG_INFO_FMT("DeleteNxlFile: '%s'", m_plainFilePath.c_str());
		}
	}

	// Return value is ignored. 
	return true;
}

bool CRMXNxlFile::AddActivityLog(RMXActivityLogOperation op, RMXActivityLogResult result, bool testNxl/* = true*/)
{
	// Sanity check
	if (testNxl && !IsValidNxl())
	{
		LOG_DEBUG_FMT(L"Invalid nxl file. Ignore to add activity log for it '%s'", m_nxlFilePath.c_str());
		return false;
	}
	
	auto pDRmUser = CRMXInstance::GetInstance().GetDRmUser();
	CHK_ERROR_RETURN(pDRmUser == nullptr, false, L"Failed to add activity log: '%s' (error: Invalid pDRmUser)", m_nxlFilePath.c_str());
	CRMXLastResult ret;
	ret = pDRmUser->AddActivityLog(m_nxlFilePath, (RM_ActivityLogOperation)op, (RM_ActivityLogResult)result);
	CHK_ERROR_RETURN(!ret, false, L"[RPM]AddActivityLog failed: '%s' (error: %s)", m_nxlFilePath.c_str(), ret.ToString().c_str());

	LOG_INFO_FMT("AddActivityLog('%s'): operation: %d, result: %d ", m_nxlFilePath.c_str(), (int)op, (int)result);
	
	return true;
}

bool CRMXNxlFile::AddActivityLogByRight(unsigned long r, RMXActivityLogResult result, bool testNxl)
{
	static std::map<unsigned long, RMXActivityLogOperation> sR2LogMap = {
		{ RMX_RIGHT_VIEW, RMX_OView },
		{ RMX_RIGHT_EDIT, RMX_OEdit },
		{ RMX_RIGHT_PRINT, RMX_OPrint },
		{ RMX_RIGHT_SCREENCAPTURE, RMX_OCaptureScreen },
		{ RMX_RIGHT_CLASSIFY, RMX_OClassify },
		{ RMX_RIGHT_SAVEAS, RMX_ODownload },
		{ RMX_RIGHT_DOWNLOAD, RMX_ODownload },
		{ RMX_RIGHT_DECRYPT, RMX_ODecrypt },
		{ RMX_RIGHT_SHARE, RMX_OShare },
		{ RMX_RIGHT_CLIPBOARD, RMX_OCopyContent}
	};

	return AddActivityLog(sR2LogMap[r], result, testNxl);
}

bool CRMXNxlFile::EnsureNxlExtension()
{
	CPathEx path(m_origFilePath.c_str());
	if (_wcsicmp(path.GetExtension().c_str(), NXL_FILE_EXT) == 0)
		return false;

	if (CPathEx::IsFile(m_nxlFilePath))
		return false;

	CRMXLastResult result;
	auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
	if (pDRmcInstance)
	{
		uint32_t dirStatus;
		bool isProtected = false;
		result = pDRmcInstance->RPMGetFileStatus(m_origFilePath, &dirStatus, &isProtected);
		CHK_ERROR(!result, L"[RPM]RPMGetFileStatus failed: '%s' (error: %s)", m_origFilePath.c_str(), result.ToString().c_str());
		
		if (!RPMHlp::IsRPMDir(dirStatus) || !isProtected)
			return false;

		if (MoveFile(m_origFilePath.c_str(), m_nxlFilePath.c_str()) == 0)
		{
			LOG_ERROR_FMT(L"EnsureNxlExtension: Failed to append .nxl extension(error: %s): %s", (LPCTSTR)CSysErrorMessage(GetLastError()), m_origFilePath.c_str());
			return false;
		}
		else
		{
			// Call FindFirstFile to notify driver there is a new NXL file
			WIN32_FIND_DATA pNextInfo;
			HANDLE hFind = FindFirstFile(m_nxlFilePath.c_str(), &pNextInfo);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				FindClose(hFind);
			}
			LOG_INFO_FMT(L"EnsureNxlExtension: .nxl extension appended: %s", m_nxlFilePath.c_str());
			return true;
		}
	}
	return false;
}

bool CRMXNxlFile::AddAttributes(unsigned long attrs)
{
	bool ret = false;
	if (IsInRPMFolder())
	{
		auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
		CHK_ERROR_RETURN(pDRmcInstance == nullptr, false, L"Failed to get file rights: '%s' (error: Invalid DRmcInstance)", m_nxlFilePath.c_str());

		CRMXLastResult result;
		std::vector<std::pair<SDRmFileRight, std::vector<SDR_WATERMARK_INFO>>> userRightsAndWatermarks;
		std::vector<SDRmFileRight> rights;
		SDR_WATERMARK_INFO waterMark;
		SDR_Expiration expiration;
		DWORD dirStatus;
		DWORD isNXL;
		string tags, duid, tokenGroup, creatorId, infoExt;
		DWORD oldAttrs;
		result = pDRmcInstance->RPMGetFileInfo(m_nxlFilePath, duid, userRightsAndWatermarks, rights, waterMark, expiration,
			tags, tokenGroup, creatorId, infoExt, oldAttrs, dirStatus, isNXL);
		CHK_ERROR_RETURN(!result || oldAttrs == INVALID_FILE_ATTRIBUTES, false, L"[RPM]RPMGetFileInfo failed: '%s' (error: %s)", m_nxlFilePath.c_str(), result.ToString().c_str());

		oldAttrs |= attrs;
		result = pDRmcInstance->RPMSetFileAttributes(m_nxlFilePath.c_str(), oldAttrs);
		CHK_ERROR_RETURN(!result, false, L"[RPM]RPMSetFileAttributes failed: '%s' (error: %s)", m_nxlFilePath.c_str(), result.ToString().c_str());
		
	}
	else
	{
		DWORD oldAttrs = GetFileAttributes(m_nxlFilePath.c_str());
		if (oldAttrs != INVALID_FILE_ATTRIBUTES)
		{
			oldAttrs |= attrs;
			ret = SetFileAttributes(m_nxlFilePath.c_str(), oldAttrs)? true : false;
			CHK_ERROR_RETURN(!ret, false, L"[RPM]SetFileAttributes failed: '%s' (error: %s)", m_nxlFilePath.c_str(), (LPCTSTR)CSysErrorMessage(GetLastError()));
		}
	}

	LOG_INFO_FMT(L"AddAttributes('%s'): %u", m_nxlFilePath.c_str(), attrs);

	return true;
}

bool CRMXNxlFile::SetAttributes(unsigned long attrs)
{
	if (IsInRPMFolder())
	{
		CRMXLastResult result;
		auto pDRmcInstance = CRMXInstance::GetInstance().GetDRmcInstance();
		CHK_ERROR_RETURN(pDRmcInstance == nullptr, false, L"Failed to get file rights: '%s' (error: Invalid DRmcInstance)", m_nxlFilePath.c_str());
		
		result = pDRmcInstance->RPMSetFileAttributes(m_nxlFilePath.c_str(), attrs);
		CHK_ERROR_RETURN(!result, false, L"[RPM]RPMSetFileAttributes failed: '%s' (error: %s)", m_nxlFilePath.c_str(), result.ToString().c_str());
	}
	else
	{
		bool ret = ::SetFileAttributes(m_nxlFilePath.c_str(), attrs) ? true : false;
		CHK_ERROR_RETURN(!ret, false, L"[RPM]SetFileAttributes failed: '%s' (error: %s)", m_nxlFilePath.c_str(), (LPCTSTR)CSysErrorMessage(GetLastError()));
	}
	
	LOG_INFO_FMT(L"SetAttributes('%s'): %u", m_nxlFilePath.c_str(), attrs);
	return true;
}

std::string CRMXNxlFile::ReadTagsFromFile() const
{
	CRMXLastResult result;
	std::string tags;
	static const wstring nxlHelperExe = _GetNXLHelperExe();
	if (nxlHelperExe.empty())
	{
		result = CRMXResult(RMX_ERROR_NXLHELPER_NOTFOUND, L"Cannot find Nxlhelper.exe in '$CREO_RMX_ROOT\tools' folder");
		LOG_ERROR(result.ToString().c_str());
		return tags;
	}
	const std::wstring cmd = L"\"" + nxlHelperExe + L"\" -jsontags \"" + m_nxlFilePath + L"\"";
	if (RMXUtils::ExecuteCmd(cmd, tags))
	{
		if (tags.empty())
			LOG_INFO_FMT(L"No tags found for nxl file '%s'", m_nxlFilePath.c_str());
		else if (tags.front() != '{')
		{
			// Print out some error like "NotExist" if file not found
			result = CRMXResult(ERROR_FILE_NOT_FOUND, RMXUtils::s2ws(tags));
			LOG_ERROR_FMT(L"Failed to read tags for nxl file '%s' (error: '%s')", m_nxlFilePath.c_str(), result.ToString().c_str());
			tags.clear();
		}
		else
			LOG_INFO_FMT(L"Read tags for nxl file('%s') : '%s'", m_nxlFilePath.c_str(), RMXUtils::s2ws(tags).c_str());
	}

	return tags;
}
