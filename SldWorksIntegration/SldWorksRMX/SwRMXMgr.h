#pragma once

#include <SDLAPI.h>
#include <map>
#include "CADRMXTypeDef.h"


#ifndef RPM_PASSCODE
#define RPM_PASSCODE "{6829b159-b9bb-42fc-af19-4a6af3c9fcf6}"
#endif

#define NXL_FILE_EXT L".nxl"


enum class FileRight : DWORD {
	RMX_RIGHT_TOOLSADDIN = 0,
	RMX_RIGHT_VIEW = 1,
	RMX_RIGHT_EDIT = 2,
	RMX_RIGHT_PRINT = 4,
	RMX_RIGHT_CLIPBOARD = 8,
	RMX_RIGHT_SAVEAS = 0x10,
	RMX_RIGHT_DECRYPT = 0x20,
	RMX_RIGHT_SCREENCAPTURE = 0x40,
	RMX_RIGHT_SEND = 0x80,
	RMX_RIGHT_CLASSIFY = 0x100,
	RMX_RIGHT_SHARE = 0x200,
	RMX_RIGHT_DOWNLOAD = 0x400,
	RMX_RIGHT_WATERMARK = 0x40000000,
	RMX_RIGHT_MODIFY = 0x80000000, //To send warning message to users for files which don't have RIGHT_EDIT permission
	RMX_RIGHT_EXPORT = (RMX_RIGHT_SAVEAS | RMX_RIGHT_DOWNLOAD)
};


class CSwRMXMgr {

private:
	ISDRmcInstance *m_pInstance;
	ISDRmTenant *m_pTenant;
	ISDRmUser *m_pUser;
	static  CSwRMXMgr * m_RMXUtilInstance;
	string m_TenantId;
	string m_MemberId;

private:
	CSwRMXMgr();
	
	~CSwRMXMgr();


public:
	CSwRMXMgr(const CSwRMXMgr &) = delete;
	CSwRMXMgr & operator=(const CSwRMXMgr &) = delete;
	bool Init();
	bool SetProcessAsTrusted();
	bool SetProcessAsTrusted(DWORD pid);
	static CSwRMXMgr * GetInstance();

	bool CheckRights(const wstring& fileName, vector<int> & fileRights);
	bool RMX_RPMGetFileStatus(const std::wstring &filename, bool* filestatus);
	bool ProtectFileAsNXL(const std::wstring fileName, std::wstring &nxlFileName, const std::wstring tags);
	bool GetTags(const std::wstring & nxlFilePath, std::wstring & tags);
	bool RMX_CopyFileUtil(const wstring &srcFile, wstring dstPath);
	bool RMX_SaveFileUtil(const std::wstring &filepath, const std::wstring& originalNXLfilePath = L"");
	bool RMX_IsRPMFolder(const std::wstring &filePath);
	bool RMX_DeleteFile(const wstring& filepath);
	bool IsRPMDir(DWORD sdrDirStatus);
	bool RMX_AddRPMFolder(const wstring& fileDir,uint32_t option = (SDRmRPMFolderOption::RPMFOLDER_API | SDRmRPMFolderOption::RPMFOLDER_OVERWRITE));
	bool RMX_RemoveRPMDir(const wstring& fileDir);
	bool ProtectFile(wstring fileName, const std::wstring tags, bool setRpmDir = false);
	bool RMX_SetViewOverlay(const vector<std::wstring> &nxlFiles, const TEvalAttributeList& contextAttrs, void * hTargetWnd);
	bool RMX_ClearViewOverlay(void* hTargetWnd);
	bool DoTagMerge(wstring & mergedfileTag);
	bool IsTagMergeRequired(const wstring& sourceFileName, const wstring& destnFileName);
	bool CopyNxlInfo(const wstring& tarFile, const wstring& srcFile, wstring& tags);
	static void ReleaseInstance();
	bool FileSaveHandler(const wstring& srcFileName, const wstring& destfileName);
	void LogFileAccessResult(const wstring &filepath, const FileRight &fileRight, const bool &authResult);
	bool RMX_GetFileReadOnly(const wstring& nxlFilePath, bool& isReadOnly);
	//bool RMX_SetFileReadOnly(const wstring& nxlFilePath, bool isReadOnly);

	bool RMX_ShowFileInfoDialog(const wstring& fileName);
	bool RMX_ShowProtectDialog(const wstring& fileName, const wstring& buttonLabel, wstring& tags);
};

#define theSwRMXMgr CSwRMXMgr::GetInstance()