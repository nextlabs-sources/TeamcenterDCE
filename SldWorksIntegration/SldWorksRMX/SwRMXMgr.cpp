#include "stdafx.h"
#include "SwRMXMgr.h"
#include "FileUtils.h"
#include "CommonUtils.h"
#include <string>
#include <PathEx.h>
#include <RMXViewOverlay.h>
#include <RMXTagHlp.h>
#include "SwUtils.h"
#include "SwResult.h"
#include "SwRMXDialog.h"
#include "SwFileTypes.h"
#include <RPMCommonDialogWrapper.hpp>

#define HAS_BIT(flags, mask) ((flags) & (mask)) == (mask)

CSwRMXMgr * CSwRMXMgr::m_RMXUtilInstance = nullptr;

static const wchar_t* NXL_HELPER_EXE = L"Nxlhelper.exe";

CSwRMXMgr::CSwRMXMgr() :m_pInstance(nullptr), m_pTenant(nullptr), m_pUser(nullptr) {
	//CSwRMXMgr Constructor
	return;
}

CSwRMXMgr::~CSwRMXMgr() {
	//CSwRMXMgr Destructor
}


bool CSwRMXMgr::Init() {
	LOG_INFO("CSwRMXMgr::Init() called");
	if (m_pInstance == nullptr || m_pTenant == nullptr || m_pUser == nullptr) {
		SDWLResult result = RPMGetCurrentLoggedInUser(string(RPM_PASSCODE), m_pInstance, m_pTenant, m_pUser);
		if (!result || m_pInstance == nullptr || m_pUser == nullptr) {
			CSwResult swResultObj;
			swResultObj.SetErrorMsg(result.ToString());
			swResultObj.SetMessageType(MsgType::MSG_ERROR);
			CSldWorksRMXDialog rmxDialogObj;
			rmxDialogObj.ShowMessageDialog(&swResultObj);
			LOG_ERROR_FMT("Failed to init RMX Manager(error: %s)", result.ToString().c_str());
			return false;
		}
		m_TenantId = m_pUser->GetSystemProjectTenantId();;
		m_MemberId = m_pUser->GetMembershipID(m_TenantId);
		LOG_INFO("SolidWorks RMX Manager started successfully");
	}
	return true;
}

bool CSwRMXMgr::SetProcessAsTrusted()
{
	wchar_t szProcessName[MAX_PATH];
	wstring ret_msg;

	if (m_pInstance == nullptr || m_pTenant == nullptr || m_pUser == nullptr) {
		LOG_ERROR("One of ISDRmcInstance/ISDRmTenant/ISDRmUser variables are null. Re-initilaizing variables again..");
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}
	
	if (GetModuleFileName(NULL, &szProcessName[0], MAX_PATH) == 0)
	{
		LOG_ERROR("Cannot open the file");
		return false;
	}

	SDWLResult result = m_pInstance->RPMRegisterApp(std::wstring(szProcessName), RPM_PASSCODE);
	ret_msg = result.ToString().c_str();
	if (!result)
	{
		LOG_ERROR_FMT ("Cannot initialize RMX. App registered unsuccessfully:%s. Error msg =  %s", szProcessName,ret_msg);
		return false;
	}

	// Inform RPM this RMX is running now.
	result = m_pInstance->RPMNotifyRMXStatus(true, RPM_PASSCODE);
	ret_msg = result.ToString().c_str();
	if (!result)
	{
		LOG_ERROR(L"Cannot initialize RMX. Unsuccessfully notify RPM status");
		return false;
	}

	return true;

}

bool CSwRMXMgr::SetProcessAsTrusted(DWORD pid)
{
	if (m_pInstance == nullptr || m_pTenant == nullptr || m_pUser == nullptr) {
		LOG_ERROR("One of ISDRmcInstance/ISDRmTenant/ISDRmUser variables are null. Re-initilaizing variables again..");
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}

	SDWLResult result = m_pInstance->RPMAddTrustedProcess(pid, RPM_PASSCODE);
	wstring ret_msg = result.ToString().c_str();
	if (!result)
	{
		LOG_ERROR_FMT("Error occured while setting process with process id as trusted. Process Id = %lu. Error Message =  %s", pid, ret_msg);
		return false;
	}
	return true;
}

CSwRMXMgr * CSwRMXMgr::GetInstance()
{
	if (m_RMXUtilInstance != nullptr) {
		return m_RMXUtilInstance;
	}
	else {
		m_RMXUtilInstance = new CSwRMXMgr();
		return m_RMXUtilInstance;
	}
}



void CSwRMXMgr::ReleaseInstance() {
	LOG_INFO("CSwRMXMgr::ReleaseInstance called.");
	//Free ISDRmcInstance instance
	if (m_RMXUtilInstance != nullptr) {
		SDWLResult result = m_RMXUtilInstance->m_pInstance->RPMNotifyRMXStatus(false, RPM_PASSCODE);
		if (!result) {
			LOG_ERROR("Failed to notify RPM status . Error:" << result.ToString().c_str());
		}
		if (m_RMXUtilInstance->m_pInstance != nullptr) {
			SDWLibDeleteRmcInstance(m_RMXUtilInstance->m_pInstance);
			m_RMXUtilInstance->m_pInstance = nullptr;
		}
		m_RMXUtilInstance->m_pUser = nullptr;
		m_RMXUtilInstance->m_pTenant = nullptr;
		delete m_RMXUtilInstance;
		m_RMXUtilInstance = nullptr;
	}
}

bool CSwRMXMgr::GetTags(const std::wstring & nxlFilePath, std::wstring& tags)
{
	if (m_pInstance == nullptr) {
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}
	
	//if file pointed by nxlFilePath is protected
	bool isProtected = false;
	if (RMX_RPMGetFileStatus(nxlFilePath, &isProtected) && isProtected) {
		SDWLResult result = m_pInstance->RPMReadFileTags(nxlFilePath, tags);
		if (!result) {
			LOG_ERROR("Error:" << result.ToString().c_str());
			return false;
		}
	}

	return true;
}




bool CSwRMXMgr::CheckRights(const wstring& fileName, vector<int> &fileRights) {

	if (m_pInstance == nullptr) {
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}

	if (m_pInstance != nullptr) {
		using rightsWatermarkPair = std::pair<SDRmFileRight, std::vector<SDR_WATERMARK_INFO>> ;
		std::vector<rightsWatermarkPair> rightsAndWatermarks;
		m_pInstance->RPMGetFileRights(fileName, rightsAndWatermarks);
		if (rightsAndWatermarks.size() == 0) {
			LOG_ERROR("No Rights are assigned");
			return false;
		}
		vector<rightsWatermarkPair>::iterator iter;
		for (iter = rightsAndWatermarks.begin(); iter != rightsAndWatermarks.end(); iter++) {
			int rightsCode = (*iter).first;
			fileRights.push_back(rightsCode);

		}
	}

	return true;

}

bool CSwRMXMgr::RMX_RPMGetFileStatus(const std::wstring &filename,bool* filestatus) {
	if (m_pInstance == nullptr) {
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}
	
	if (m_pInstance != nullptr) {
		unsigned int dirstatus;
		SDWLResult result = m_pInstance->RPMGetFileStatus(filename, &dirstatus, filestatus);
		if (!result) {
			LOG_ERROR("Error:" << result.ToString().c_str());
			return false;
		}
	}
	else {
		LOG_ERROR("m_pInstance couldn't initialized");
		return false;
	}

	return true;
}


bool CSwRMXMgr::ProtectFileAsNXL(const std::wstring fileName, std::wstring &nxlFileName,const std::wstring tags)
{
	if (m_pUser == nullptr) {
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}

	if (m_pUser != nullptr) {

		nxlFileName = FileUtils::GetParentPath(fileName);
		vector<SDRmFileRight> rights;
		SDR_Expiration expire;
		SDR_WATERMARK_INFO watermarkinfo;
		expire.type = IExpiryType::NEVEREXPIRE;
		string normalizedTags;
		RMXLib::MergeTags(m_pUser, RMXUtils::ws2s(tags), normalizedTags);
		
		SDWLResult result = m_pUser->ProtectFile(fileName, nxlFileName, rights, watermarkinfo, expire, normalizedTags, m_MemberId);
		if (!result) {
			LOG_ERROR("Error:" << result.ToString().c_str());
			return false;
		}
	}
	else {
		LOG_ERROR("m_pUser not initialized");
		return false;
	}
	return true;
}



bool CSwRMXMgr::RMX_CopyFileUtil(const wstring &srcFile,wstring dstPath) {
	LOG_INFO_FMT("srcFile=%s,dstPath=%s",srcFile.c_str(),dstPath.c_str());
	if (m_pInstance == nullptr) {
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}

	if (m_pInstance != nullptr) {
		SDWLResult result = m_pInstance->RPMEditCopyFile(srcFile, dstPath);
		if (!result) {
			LOG_ERROR("Error :" << result.ToString().c_str());
			return false;
		}
	}	
	return true;
}

bool CSwRMXMgr::RMX_SaveFileUtil(const std::wstring &plainFilePath, const std::wstring& nxlFilePath) {
	LOG_INFO_FMT("plainFilePath=%s,nxlFilePath=%s", plainFilePath.c_str(), nxlFilePath.c_str());
	if (m_pInstance == nullptr) {
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}
	if (m_pInstance != nullptr) {
		SDWLResult result = m_pInstance->RPMEditSaveFile(plainFilePath, nxlFilePath,false,3);
		if (!result) {
			LOG_ERROR("Error :" << result.ToString().c_str());
			return false;
		}
	}
	return true;
}


bool CSwRMXMgr::RMX_DeleteFile(const wstring& filepath) {
	LOG_INFO("File to delete :" << filepath.c_str());

	wstring fileDir = FileUtils::GetParentPath(filepath);
	bool isProtected = false;
	if (RMX_IsRPMFolder(fileDir) && RMX_RPMGetFileStatus(filepath, &isProtected) && isProtected) {

		if (m_pInstance == nullptr) {
			if (!Init()) {
				LOG_ERROR("Error initializing member variables.");
				return false;
			}
		}

		if (m_pInstance != nullptr) {
			SDWLResult result = m_pInstance->RPMDeleteFile(filepath);
			if (!result) {
				LOG_ERROR("Error = " << result.ToString().c_str());
				return false;
			}
		}
	}
	else {
		bool status = FileUtils::DeleteFileUtil(filepath.c_str());
		if (!status) {
			LOG_ERROR("Error deleting file = " << filepath.c_str());
			//Force delete a file if it is read-only;
			status = FileUtils::ForceDelete(filepath.c_str());
			if(!status)
				return false;
		}
	}
	return true;

}


bool CSwRMXMgr::RMX_IsRPMFolder(const std::wstring &filePath ) {
	unsigned int dirstatus = 0;
	if (m_pInstance == nullptr) {
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}
	if (m_pInstance != nullptr) {
		SDRmRPMFolderOption option;
		std::wstring fileTags;
		SDWLResult result = m_pInstance->IsRPMFolder(filePath, &dirstatus, &option, fileTags);
		if (!result) {
			LOG_ERROR("Error :" << result.ToString().c_str());
			return false;
		}
	}

	bool isRPMDir = IsRPMDir(dirstatus);
	//LOG_INFO_FMT(L"Is RPM folder('%s'): %s (Relation: %d)", filePath.c_str(), isRPMDir ? L"Yes" : L"No", dirstatus);
	
	// The nxl file extension is hidden in safe dir or posterity of safe dir
	return isRPMDir;
	

}

bool CSwRMXMgr::IsRPMDir(DWORD sdrDirStatus)
{
	return (sdrDirStatus & RPM_SAFEDIRRELATION_SAFE_DIR || sdrDirStatus & RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR);
}


bool CSwRMXMgr::RMX_AddRPMFolder(const wstring& dirPath,uint32_t option){
	LOG_INFO("Directory Name = " << dirPath.c_str());
	
	if (m_pInstance == nullptr) {
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}

	if (m_pInstance != nullptr) {		
		SDWLResult result = m_pInstance->AddRPMDir(dirPath,option);
		if (!result) {
			LOG_ERROR("Error :" << result.ToString().c_str());
			return false;
		}
	}
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

	LOG_INFO(L"RPM dir added = " << dirPath.c_str());
	return true;


}


bool CSwRMXMgr::RMX_RemoveRPMDir(const wstring& dirPath) {
	LOG_INFO("Directory Name = " << dirPath.c_str());

	if (m_pInstance == nullptr) {
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}

	if (m_pInstance != nullptr) {
		SDWLResult result = m_pInstance->RemoveRPMDir(dirPath, true);
		if (!result) {
			LOG_ERROR("Error :" << result.ToString().c_str());
			return false;
		}
	}
	LOG_INFO(L"RPM dir removed: " << dirPath.c_str());
	return true;
}

bool CSwRMXMgr::ProtectFile(wstring fileName,const std::wstring fileTags, bool setRpmDir) {
	LOG_INFO("FileName = " << fileName.c_str());
	//Check if the file still exists
	if (!FileUtils::IsFileExists(fileName)) {
		//Reason for file non existence could be due to SolidWorks delete the file as in case of Creo Part File
		LOG_ERROR("File doesn't exists = " << fileName.c_str());
		return false;
	}
	if (setRpmDir) {
		//Check if the CWD is a RPM folder.
		wstring fileDir = FileUtils::GetParentPath(fileName);
		if (!RMX_IsRPMFolder(fileDir)) {
			LOG_INFO("Setting directory as RPM. FileDirectory = " << fileDir);
			if (!RMX_AddRPMFolder(fileDir, (SDRmRPMFolderOption::RPMFOLDER_NORMAL | SDRmRPMFolderOption::RPMFOLDER_OVERWRITE))) {
				LOG_WARN("Failed to set directory as RPM = " << fileDir);
			}
		}
	}

	wstring nxlFileName;
	bool res = ProtectFileAsNXL(fileName, nxlFileName,fileTags);
	if (res && FileUtils::IsFileExists(nxlFileName)) {
		LOG_INFO("File exists = " << nxlFileName.c_str());
		//Delete fileName
		res = RMX_DeleteFile(fileName.c_str());
		if (res) {
			//Rename nxlFileName to fileName;

			//if not rpm dir then append .nxl to the fileName
			if (!RMX_IsRPMFolder(FileUtils::GetParentPath(fileName))) {
				wstring fileNameWithoutExt = FileUtils::GetFileName(fileName);
				wstring fileExt = FileUtils::GetFileExt(fileName);
				int fileCopyCount = 0;
				wstring newFileName = fileName + NXL_FILE_EXT;
				while (FileUtils::IsFileExists(newFileName))
				{ 
					++fileCopyCount;
					newFileName = fileNameWithoutExt + L"(" + to_wstring(fileCopyCount) + L")" + fileExt + NXL_FILE_EXT;
				}
				fileName = newFileName;
			}

			if (!MoveFile(nxlFileName.c_str(), fileName.c_str())) {
				LOG_ERROR_FMT("Error moving file. Old File Name = %s, New File Name = %s, GetLastError = %d", nxlFileName.c_str(), fileName.c_str(), GetLastError());
				return false;
			}

		}
		else {
			LOG_ERROR("Error occured while deleting the file = " << fileName.c_str());
			return false;
		}
	}
	else {
		LOG_ERROR("Error applying protection to the file = " << fileName.c_str());
		return false;
	}

	return true;

}


bool CSwRMXMgr::RMX_SetViewOverlay(const vector<std::wstring> &nxlFiles, const TEvalAttributeList& contextAttrs, void * hTargetWnd) {
	if (m_pInstance == nullptr) {
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}
	return RMXLib::SetViewOverlay(m_pInstance, hTargetWnd, nxlFiles, contextAttrs);
}

bool CSwRMXMgr::RMX_ClearViewOverlay(void* hTargetWnd) {
	if (m_pInstance == nullptr) {
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}

	return RMXLib::ClearViewOverlay(m_pInstance, hTargetWnd);
}


bool CSwRMXMgr::DoTagMerge(wstring & mergedFileTag) {
	LOG_INFO("CSwRMXMgr::DoTagMerge called");
	bool isProtected = false;
	CSwUtils * swInstance = CSwUtils::GetInstance();
	wstring activeFileName = swInstance->GetCurrentActiveFile();
	wstring fileTag = L"";
	if (!activeFileName.empty()
		&& RMX_RPMGetFileStatus(activeFileName, &isProtected) && isProtected) {
		GetTags(activeFileName, fileTag);
		mergedFileTag = mergedFileTag + fileTag;
	}
	vector<wstring> referencedFileNames = swInstance->GetReferencedModelDocuments();
	for (auto fileName : referencedFileNames) {
		isProtected = false;
		fileTag = L"";
		if (RMX_RPMGetFileStatus(fileName, &isProtected) && isProtected) {
			GetTags(fileName, fileTag);
			mergedFileTag = mergedFileTag + fileTag;
		}
	}
	return true;
}
bool CSwRMXMgr::CopyNxlInfo(const wstring& tarFile, const wstring& srcFile, wstring& tags) {
	bool ret, isSrcProtected, isTarProtected;
	wstring srcTags, tarTags;
	//get file status and tags of source file
	ret = RMX_RPMGetFileStatus(srcFile, &isSrcProtected);
	if (!ret)
	{
		LOG_ERROR_FMT("RMX_RPMGetFileStatus Error(file-'%s')", srcFile.c_str());
		return false;
	}
	if (!isSrcProtected) {
		LOG_DEBUG_FMT("SourceFile('%s'):isProtected=false'", srcFile.c_str());
		return false;
	}
	ret = GetTags(srcFile, srcTags);
	if (!ret)
	{
		LOG_ERROR_FMT("GetTags Error(file-'%s')", srcFile.c_str());
		return false;
	}
	LOG_DEBUG_FMT("SourceFile('%s'):isProtected=true, tags='%s'", srcFile.c_str(), srcTags.c_str());


	//check file status and tags of target file
	ret = RMX_RPMGetFileStatus(tarFile, &isTarProtected);
	if (!ret)
	{
		LOG_ERROR_FMT("RMX_RPMGetFileStatus Error(file-'%s')", tarFile.c_str());
		return false;
	}
	if (isTarProtected) {
		ret = GetTags(tarFile, tarTags);
		LOG_DEBUG_FMT("TargetFile('%s'):isProtected=true, tags='%s'", tarFile.c_str(), tarTags.c_str());
		tags = tarTags;
		return false;
	}
	LOG_DEBUG_FMT("TargetFile('%s'):isProtected=false'", tarFile.c_str());
	//protect the tmp file
	if (ProtectFile(tarFile, srcTags, true))
	{
		tags = srcTags;
		return true;
	}
	return false;
}

bool CSwRMXMgr::IsTagMergeRequired(const wstring& sourceFileName, const wstring& destnFileName) {
	LOG_INFO_FMT("Source File = %s , Destination File = %s ", sourceFileName.c_str(), destnFileName.c_str());
	if (SwFileTypes::IsAutoRecoverFile(destnFileName)) {
		return false;
	}
	long fromDocType = sourceFileName.empty()
		? CSwUtils::GetInstance()->GetModelDocType()
		: CSwUtils::GetInstance()->GetModelDocTypeByName(sourceFileName);
	long toDocType = CSwUtils::GetInstance()->GetModelDocTypeByName(destnFileName);
	if ((fromDocType == swDocASSEMBLY && (toDocType != swDocASSEMBLY && toDocType != swDocDRAWING))
		|| (fromDocType == swDocDRAWING && toDocType != swDocDRAWING)) {
		LOG_INFO("Tag merged required for the destination file. Destination File" << destnFileName.c_str());
		return true;
	}
	return false;
}


bool CSwRMXMgr::FileSaveHandler(const wstring& srcFileName, const wstring& destfileName) {
	LOG_INFO_FMT("Source File Name = %s , Destination File Name = %s", srcFileName.c_str(), destfileName.c_str());
	//Protect the file here
	wstring fileTag = L"";
	bool isTagMerged = IsTagMergeRequired(srcFileName, destfileName);
	if (isTagMerged) {
		DoTagMerge(fileTag);
	}
	else if(!srcFileName.empty()) {
		GetTags(srcFileName, fileTag);
	}
	if (!fileTag.empty()) {
		LOG_INFO("File Protection Begin :" << destfileName);
		LOG_INFO_FMT("SourceFileName=%s , TargetFileName = %s , IsTagMerged = %d", srcFileName.c_str(), destfileName.c_str(), isTagMerged);
		if (!ProtectFile(destfileName, fileTag)) {
			LOG_ERROR("Failed to protect the file = " << destfileName);
			return false;
		}
		LOG_INFO("File Protection End :" << destfileName);
	}
	return true;
}

void CSwRMXMgr::LogFileAccessResult(const wstring &filepath, const FileRight &fileRight, const bool &authResult) {
	static std::map<FileRight, RM_ActivityLogOperation> sR2LogMap = {
		{ FileRight::RMX_RIGHT_VIEW, RL_OView },
		{ FileRight::RMX_RIGHT_EDIT, RL_OEdit },
		{ FileRight::RMX_RIGHT_PRINT, RL_OPrint },
		{ FileRight::RMX_RIGHT_SCREENCAPTURE, RL_OCaptureScreen },
		{ FileRight::RMX_RIGHT_CLASSIFY, RL_OClassify },
		{ FileRight::RMX_RIGHT_CLIPBOARD, RL_OCopyContent },
		{ FileRight::RMX_RIGHT_DECRYPT, RL_ODecrypt },
		{ FileRight::RMX_RIGHT_SHARE, RL_OShare },
		{ FileRight::RMX_RIGHT_SAVEAS, RL_ODownload }
	};
	auto iter = sR2LogMap.find(fileRight);
	if (iter == sR2LogMap.end())
	{
		LOG_ERROR_FMT("Activity logging not supported for the received file right(FilePath = %s, FileRight = %lu, isAllowed = %d)", filepath.c_str(), static_cast<DWORD>(fileRight), authResult);
		return;
	}
	
	if (m_pUser == nullptr) {
		if (!Init()) {
			LOG_ERROR("Error initializing member variables.");
			return ;
		}
	}
	m_pUser->AddActivityLog(filepath, (*iter).second, authResult ? RL_RAllowed : RL_RDenied);
	LOG_INFO_FMT("Activity log added(FilePath = %s , Operation = %d, Result = %d)", filepath.c_str(), (*iter).second, authResult);
}

bool CSwRMXMgr::RMX_GetFileReadOnly(const wstring & nxlFilePath, bool& isReadOnly)
{
	LOG_DEBUG(L"RMX_GetFileReadOnly entering:" << nxlFilePath.c_str());
	if (m_pInstance == nullptr) {
		if (!Init() || m_pInstance == nullptr) {
			LOG_ERROR("Error initializing member variables.");
			return false;
		}
	}
	// Request to pass a nxl file path
	CPathEx filePath(nxlFilePath.c_str());
	if (_wcsicmp(filePath.GetExtension().c_str(), NXL_FILE_EXT) != 0)
		filePath += NXL_FILE_EXT;

	std::string duid;
	std::vector<std::pair<SDRmFileRight, std::vector<SDR_WATERMARK_INFO>>> userRightsAndWatermarks;
	std::vector<SDRmFileRight> rights;
	SDR_WATERMARK_INFO watermark;
	SDR_Expiration expiration;
	std::string tags;
	std::string tokengroup;
	std::string creatorid;
	std::string infoext;
	DWORD attributes = INVALID_FILE_ATTRIBUTES;
	DWORD isRPMFolder;
	DWORD isNxlFile;

	auto result = m_pInstance->RPMGetFileInfo(filePath.ToString(),
		duid, userRightsAndWatermarks, rights, watermark, expiration, tags, tokengroup, creatorid, infoext,
		attributes, isRPMFolder, isNxlFile);

	if (!result) {
		LOG_ERROR_FMT("RPMGetFileInfo failed(error: %s): ", result.ToString().c_str(), filePath.c_str());
		return false;
	}

	isReadOnly = false;
	if ((attributes != INVALID_FILE_ATTRIBUTES) && HAS_BIT(attributes, FILE_ATTRIBUTE_READONLY))
	{
		isReadOnly = true;
	}

	return true;
}

bool CSwRMXMgr::RMX_ShowFileInfoDialog(const wstring& fileName)
{
	RPMCommonDialogWrapper dlgWrapper;
	return dlgWrapper.ShowFileInfoDialog(fileName.c_str(), L"");
}

bool CSwRMXMgr::RMX_ShowProtectDialog(const wstring& fileName, const wstring& buttonLabel, wstring& tags)
{
	RPMCommonDialogWrapper dlgWrapper;
	return dlgWrapper.ShowProtectDialog(fileName.c_str(), buttonLabel.c_str(), tags);
}

//bool CSwRMXMgr::RMX_SetFileReadOnly(const wstring & nxlFilePath, bool isReadOnly)
//{
//	LOG_DEBUG(L"RMX_SetFileReadOnly entering:" << nxlFilePath.c_str() << L", isReadOnly: " << isReadOnly);
//
//	bool alreadyReadOnly = false;
//	if (!RMX_GetFileReadOnly(nxlFilePath, alreadyReadOnly))
//		return false;
//
//	if (m_pInstance == nullptr) {
//		if (!Init() || m_pInstance == nullptr) {
//			LOG_ERROR("Error initializing member variables.");
//			return false;
//		}
//	}
//
//	// Request to pass a nxl file path
//	CPathEx filePath(nxlFilePath.c_str());
//	if (_wcsicmp(filePath.GetExtension().c_str(), NXL_FILE_EXT) != 0)
//		filePath += NXL_FILE_EXT;
//		
//	if (alreadyReadOnly != isReadOnly)
//	{
//		auto result = m_pInstance->RPMSetFileAttributes(filePath.ToString(), FILE_ATTRIBUTE_READONLY);
//		if (!result) {
//			LOG_ERROR_FMT("RPMSetFileAttributes failed(error: %s): %s", result.ToString().c_str(), filePath.c_str());
//			return false;
//		}
//
//		LOG_INFO_FMT(L"RMX_SetFileReadOnly succeeded. FilePath = %s, isReadOnly = %d", nxlFilePath.c_str(), isReadOnly);
//	}
//	else
//	{
//		LOG_INFO_FMT(L"RMX_SetFileReadOnly ignored. File already read-only. FilePath = %s", nxlFilePath.c_str());
//	}
//	return true;
//}
