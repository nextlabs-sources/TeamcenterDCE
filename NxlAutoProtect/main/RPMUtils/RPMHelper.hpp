#pragma once
#include <SDWL\SDLAPI.h>
#include <Windows.h>
#include <string>

#define RPM_MAGIC_SECURITY "{6829b159-b9bb-42fc-af19-4a6af3c9fcf6}"

#define RPM_CALL(x) VerifyRPMResult(x, __FUNCTION__, __FILE__, __LINE__, #x)
class RPMHelper {
public:
	RPMHelper():_passcode(RPM_MAGIC_SECURITY) {
		RPM_CALL(RPMGetCurrentLoggedInUser(_passcode, _pInstance, _pTenant, _pUser));
	}
	enum RPMFolderStatus { IsRPMDir, AncestorOfRPMDir, PosterityOfRPMDir, NonRPMDir, UnknownStatus };
	RPMFolderStatus FolderStatus(const unsigned int status) {
		if (status == 0) {
			return NonRPMDir;
		}
		if (status & RPM_SAFEDIRRELATION_SAFE_DIR) {
			return IsRPMDir;
		}
		if (status & RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR) {
			return PosterityOfRPMDir;
		}
		if (status & RPM_SAFEDIRRELATION_ANCESTOR_OF_SAFE_DIR) {
			return AncestorOfRPMDir;
		}
		return UnknownStatus;
	}
	bool IsAppRegistered(const std::wstring &appPath, bool &registered) {
		if (!IsUserLoggedIn())
			return false;

		return RPM_CALL(_pInstance->RPMIsAppRegistered(appPath, registered));
	}
	bool AddRPMFolder(const std::wstring &folderPath, uint32_t option = (SDRmRPMFolderOption::RPMFOLDER_NORMAL | SDRmRPMFolderOption::RPMFOLDER_OVERWRITE))
	{
		if (!IsUserLoggedIn())
			return false;

		std::vector<std::pair<std::wstring, std::wstring>> renamedFiles;
		DBGLOG("AddRPMDir('%s')...", folderPath.c_str());
		//if (RPM_CALL(_pInstance->AddRPMDir(folderPath, FCRAutoRenameNxlFile, &renamedFiles)))
		if (RPM_CALL(_pInstance->AddRPMDir(folderPath, option)))
		{
			return false;
		}
		if (renamedFiles.size() > 0)
		{
			for (int i = 0; i<renamedFiles.size(); i++)
			{
				DBGLOG("Renamed[%d/%d]'%s'==>'%s'", i + 1, renamedFiles.size(), renamedFiles[i].first.c_str(), renamedFiles[i].second.c_str());
			}
		}
		return true;
	}
	bool RemoveRPMFolder(const std::wstring &folderPath, bool force=true)
	{
		if (!IsUserLoggedIn())
			return false;

		DBGLOG("RemoveRPMDir('%s', force=%d)...", folderPath.c_str(), force);
		return RPM_CALL(_pInstance->RemoveRPMDir(folderPath, force));
	}
	bool CheckFileStatus(const std::wstring &filePath, bool *oIsProtected, RPMFolderStatus *oFolderStatus)
	{
		if (!IsUserLoggedIn())
			return false;
		unsigned int dirStatus;
		if (RPM_CALL(_pInstance->RPMGetFileStatus(filePath, &dirStatus, oIsProtected)))
		{
			*oFolderStatus = FolderStatus(dirStatus);
			return true;
		}
		return false;
	}
	bool RegisterApp(const wchar_t *app = nullptr)
	{
		if (!IsUserLoggedIn())
			return false;

		//register application
		std::wstring appPath;
		if (app == nullptr||wcslen(app)==0) {
			wchar_t buffer[MAX_PATH];
			GetModuleFileName(NULL, buffer, MAX_PATH);
			appPath = buffer;
		}
		else
		{
			appPath = app;
		}
		DBGLOG("RPMRegisterApp('%s')...", appPath.c_str());
		return RPM_CALL(_pInstance->RPMRegisterApp(appPath, _passcode));
	}
	bool SetRMXStatus(bool enable) {
		if (!IsUserLoggedIn())
			return false;
		//register self
		if(enable)
			RegisterApp();
		DBGLOG("RPMNotifyRMXStatus(%d)", enable);
		return RPM_CALL(_pInstance->RPMNotifyRMXStatus(enable, _passcode));
	}
	bool SetTrustedProcess(DWORD pid, bool isTrust, bool currentRMXStatus = false) {
		if (!IsUserLoggedIn())
			return false;

		bool success = false;
		//register self
		RegisterApp();
		if (!currentRMXStatus)
			RPM_CALL(_pInstance->RPMNotifyRMXStatus(true, _passcode));
		if (isTrust)
		{
			success = RPM_CALL(_pInstance->RPMAddTrustedProcess(pid, _passcode));
			DBGLOG("RPMAddTrustedProcess(%lu)", pid);
		}
		else
		{
			//TODO:workaround:AddTrustedProcess will clear the cache for the files has been opened in this process;
			success = RPM_CALL(_pInstance->RPMAddTrustedProcess(pid, _passcode));
			DBGLOG("RPMAddTrustedProcess(%lu)", pid);
			success = RPM_CALL(_pInstance->RPMRemoveTrustedProcess(pid, _passcode));
			DBGLOG("RPMRemoveTrustedProcess(%lu)", pid);
		}
		if (!currentRMXStatus)
			RPM_CALL(_pInstance->RPMNotifyRMXStatus(false, _passcode));
		return success;
	}
	inline bool IsUserLoggedIn()
	{
		return _pUser != nullptr;
	}
private:
	bool VerifyRPMResult(const SDWLResult &ret, const char *func, const char *file, int line, const char *call) {
		if (ret != 0)
		{
			DBGLOG("RPMAPI Failed(Code=%d(%#X) Message='%S' %S()@file-%S)", ret.GetCode(), ret.GetCode(), ret.GetMsg().c_str(), ret.GetFunction(), ret.GetFile());
			return false;
		}
		return true;
	}
	std::string _passcode;
	ISDRmTenant *_pTenant;
	ISDRmUser *_pUser;
	ISDRmcInstance *_pInstance;
};