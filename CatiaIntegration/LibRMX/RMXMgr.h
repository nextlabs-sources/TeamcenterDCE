#pragma once

#include <string>
#include <vector>

#include <CommonTypes.h>
#include "RMXTypes.h"
#include <CADRMXTypeDef.h>

class CRMXMgr
{
	RMX_SINGLETON_DECLARE(CRMXMgr)

public:
	bool ProtectFile(const std::string & tags, const std::wstring& filePath, const std::wstring& newNxlFilePath);
	bool IsRPMFolder(const std::wstring& dirPath, RMX_SAFEDIRRELATION* dirRelation = nullptr);
	bool AddRPMDir(const std::wstring& dirPath, uint32_t option = RMX_RPMFOLDER_OVERWRITE | RMX_RPMFOLDER_API);
	bool RemoveRPMDir(const std::wstring& dirPath);
	bool RPMRegisterApp(const std::wstring& appPath);
	bool RPMUnregisterApp(const std::wstring& appPath);
	bool RPMAddTrustedProcess(unsigned long processId);
	bool MergeTags(const std::string& tags, std::string& normalizedTags);
	bool RPMSetViewOverlay(std::vector<std::wstring>& vecFiles, const TEvalAttributeList& contextAttrs, void* hTargetWnd,
		const std::tuple<int, int, int, int>& displayOffset);
	bool RPMClearViewOverlay(void* hTargetWnd);
	bool RPMAddTrustedApp(const std::wstring& appPath);
	bool RPMRemoveTrustedApp(const std::wstring& appPath);
	bool ReprotectFile(const std::wstring& originalNxlFilePath);
	bool RPMEditSaveFile(const std::wstring& filepath, const std::wstring& originalNXLfilePath = L"", bool deletesource = false, RMXEditSaveMode mode = RMX_EditSaveMode_None);
	void NotifyMessage(const std::wstring& filePath, const std::wstring& msg);
};

