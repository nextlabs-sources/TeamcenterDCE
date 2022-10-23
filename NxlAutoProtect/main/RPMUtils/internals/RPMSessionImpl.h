#pragma once
#include <stdafx.h>
#include <RPMUtils.h>
#include <internals/Utilities.h>
#include <internals/NxlMetadataPriorities.h>
#include <NxlUtils/NxlHelper.exe.hpp>


#ifndef RPM_PASSCODE
#define RPM_PASSCODE "{6829b159-b9bb-42fc-af19-4a6af3c9fcf6}"
#endif

ResultInfo GetWinErr(const char *file, const char*func, int line)
{
	DWORD error = GetLastError();
	if (error == 0) return ResultInfo();
	char *errorMessage = NULL;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorMessage, 0, NULL);
	TRACELOG("GetLastError()=%lu Message='%S'", error, errorMessage);
	auto ret = ResultInfo(error, errorMessage, func, file, line);
	LocalFree(errorMessage);
	return ret;
}

ResultInfo report_rpm_error(const SDWLResult &ret, const char *func, const char *file, int line, const char *call)
{
	if (ret == (int)SDWL_SUCCESS)
	{
		return ResultInfo();
	}
	//TODO:logging
	ERRLOG("%S():RPMAPI Failed(Code=%d(%#X) Message='%S' %S()@file-%S)", func, ret.GetCode(), ret.GetCode(), ret.GetMsg().c_str(), ret.GetFunction(), ret.GetFile());
	ResultInfo rpmError(ret.GetCode(), ret.GetMsg().c_str(), ret.GetFunction(), ret.GetFile(), ret.GetLine());
	return ResultInfo(rpmError, ret.GetCode(), call, func, file, line);
}

#define RPM_CALL(x) (_lastError = report_rpm_error(x, __FUNCTION__, __FILE__, __LINE__, #x))

class RPMSessionImpl {
public:
	friend class RPMSession;
	RPMSessionImpl(const char *passcode) :_passcode(passcode), _initialized(false), _rmcInstance(nullptr)
	{
	}
	~RPMSessionImpl()
	{
		reset();
	}
	void reset() {
		_initialized = false;
		if (_rmcInstance != nullptr)
		{
			TRACELOG("SDWLibDeleteRmcInstance(%p)...", _rmcInstance);
			SDWLibDeleteRmcInstance(_rmcInstance);
			_rmcInstance = nullptr;
		}
		_pTenant = nullptr;
		_puser = nullptr;
	}
	bool Initialize(bool force = false)
	{
		_lastError.clear();
		if (!_initialized || force)
		{
			reset();
			TRACELOG("RPMGetCurrentLoggedInUser...");
			_initialized = RPM_CALL(RPMGetCurrentLoggedInUser(_passcode, _rmcInstance, _pTenant, _puser)) == 0;
		}
		return _initialized;
	}
	bool set_trusted_process(DWORD pid, bool isadd)
	{
		if (isadd)
		{
			TRACELOG("RPMAddTrustedProcess(%lu)...", pid);
			return RPM_CALL(_rmcInstance->RPMAddTrustedProcess(pid, _passcode)) == 0;
		}
		else
		{
			//TODO:workaround:AddTrustedProcess will clear the cache for the files has been opened in this process;
			TRACELOG("RPMAddTrustedProcess(%lu)...", pid);
			RPM_CALL(_rmcInstance->RPMAddTrustedProcess(pid, _passcode));
			TRACELOG("RPMRemoveTrustedProcess(%lu)...", pid);
			return RPM_CALL(_rmcInstance->RPMRemoveTrustedProcess(pid, _passcode)) == 0;
		}
	}
	bool register_application(bool isReg, const NxlPath &appPath) {
		if (isReg)
		{
			TRACELOG("RPMRegisterApp('%s')...", appPath.tstr());
			return RPM_CALL(_rmcInstance->RPMRegisterApp(appPath.wpath(), _passcode)) == 0;
		}
		else
		{
			TRACELOG("RPMUnregisterApp('%s')...", appPath.tstr());
			return RPM_CALL(_rmcInstance->RPMUnregisterApp(appPath.wpath(), _passcode)) == 0;
		}
	}
	bool is_application_registered(const NxlPath &app) {
		bool registered;
		TRACELOG("RPMIsAppRegistered('%s')...", app.tstr());
		if (RPM_CALL(_rmcInstance->RPMIsAppRegistered(app.wpath(), registered)) == 0)
		{
			return registered;
		}
		return false;
	}
	bool rmx_set_status(bool status)
	{
		TRACELOG("RPMNotifyRMXStatus(%d)...", status);
		return RPM_CALL(_rmcInstance->RPMNotifyRMXStatus(status, _passcode)) == 0;
	}

	bool user_is_file_protected(const NxlPath &file)
	{
		return _puser->IsFileProtected(file.wpath());
	}
	inline bool file_status(const NxlPath &file, bool *oIsProtected, RPMSession::RPMFolderStatus *oFolderStatus=nullptr)
	{
		unsigned int status;
		if (RPM_CALL(_rmcInstance->RPMGetFileStatus(file.wpath(), &status, oIsProtected)) == 0)
		{
			if (oFolderStatus != nullptr)
			{
				*oFolderStatus = FolderStatus(status);
			}
			return true;
		}
		return false;
	}
	bool rpm_get_rights(const NxlPath &file, std::vector<tstring> &rightNames) {
		std::vector<std::pair<SDRmFileRight, std::vector<SDR_WATERMARK_INFO>>> rightsAndWatermark;
		if (RPM_CALL(_rmcInstance->RPMGetFileRights(file.wpath(), rightsAndWatermark)) == 0)
		{
			for (auto r : rightsAndWatermark) {
				rightNames.push_back(FileRight_tostring(r.first));
				if (r.first == RIGHT_VIEW) {
					DBGLOG("watermark=%d", r.second.size());
				}
			}
			return true;
		}
		return false;
	}
	bool CheckRightForFiles(const std::vector<std::wstring> &files, SDRmFileRight right) {
		std::vector<std::pair<SDRmFileRight, std::vector<SDR_WATERMARK_INFO>>> rightsAndWatermark;
		bool ret = true;
		for (auto file = files.begin(); file != files.end(); file++)
		{
			if (!_puser->IsFileProtected(*file)) {
				TRACELOG("'%s':not protected", file->c_str());
				continue;
			}
			if (RPM_CALL(_rmcInstance->RPMGetFileRights(*file, rightsAndWatermark)) == 0)
			{
				bool contains = false;
				tostringstream ss;
				for (auto r : rightsAndWatermark) {
					auto rightName = FileRight_tostring(r.first);
					ss << rightName << TEXT(";");
					if (r.first == right) {
						contains = true;
						TRACELOG("'%s':'%s' MATCHED", file->c_str(), rightName.c_str());
						break;
					}
				}
				if (!contains)
				{
					//VETO
					TRACELOG("'%s':'%s'", file->c_str(), ss.str().c_str());
					ret = false;
					break;
				}
			}
		}
		return ret;
	}
	bool user_read_metadata(const NxlPath &file, NxlMetadataCollection &tags)
	{
		ISDRmNXLFile *nxlFile;
		if (RPM_CALL(_puser->OpenFile(file.wpath(), &nxlFile)) != 0)
		{
			if (_lastError.Code() == SDWL_ACCESS_DENIED)
			{
				//user don't have read right
			}
			return false;
		}
		tags = NxlMetadataCollection(nxlFile->GetTags().c_str());
		RPM_CALL(_puser->CloseFile(nxlFile));
		_lastError.clear();
		return true;
	}
	bool helper_is_file_protected(const NxlPath &file, bool *oIsProtected)
	{
		NxlHelper helper;
		if (helper.IsFileProtected(file, oIsProtected))
		{
			return true;
		}
		else
		{
			SetError(ResultInfo(helper.error(), "NxlHelper.exe failed", __FUNCTION__, __FILE__, __LINE__));
			return false;
		}
	}
	bool helper_read_metadata(const NxlPath &file, NxlMetadataCollection &tags, bool *oIsProtected = nullptr)
	{
		std::string jsonTag;
		bool isprotected;
		NxlHelper helper;
		if (helper.GetJasonTags(file, jsonTag, &isprotected))
		{
			tags = NxlMetadataCollection(jsonTag.c_str());
			if (oIsProtected != nullptr) {
				*oIsProtected = isprotected;
			}
			return true;
		}
		else
		{
			SetError(ResultInfo(helper.error(), "NxlHelper.exe failed", __FUNCTION__, __FILE__, __LINE__));
		}
		return false;
	}
	std::string user_membershipID() {
		bool adhoc;
		bool workspace = false;
		int heartbeat;
		int sysProjectId;
		std::string sysProjectTenantId, tenantId;
		if (_puser->GetTenantPreference(&adhoc, &workspace, &heartbeat, &sysProjectId, sysProjectTenantId, tenantId) != 0)
		{
			return std::string();
		}
		std::string midSysProj = _puser->GetMembershipID(sysProjectId);
		if (midSysProj.empty())
			return _puser->GetMembershipID(sysProjectTenantId);
		return midSysProj;
	}
	std::vector<tstring> user_get_policies() {
		std::string bundle;
		std::vector<tstring> policies;
		DBGLOG("GetPolicyBundle API is removed from Lhotse release, since now all the policy bundle PDP get is from CC directly");
		/*
		if (_puser->GetPolicyBundle(_pTenant->GetTenant(), bundle))
		{
			tostringstream stream;
			bool eol = false;
			for (auto c : bundle) {
				switch (c) {
				case '\r':
					break;
				case '\n':
					if (eol)
					{
						policies.push_back(stream.str());
						stream.str(TEXT(""));
					}
					else
					{
						eol = true;
						stream << c;
					}
					break;
				default:
					eol = false;
					stream << c;
					break;
				}
			}
			policies.push_back(stream.str());
		}
		*/
		return policies;
	}
	NxlPath user_protect_file(const NxlPath &file, const char *tagstring)
	{
		std::wstring newName;
		std::vector<SDRmFileRight> rights;
		SDR_WATERMARK_INFO watermark;
		SDR_Expiration expire;
		const std::string userMemberId = user_membershipID();
		if (RPM_CALL(_puser->ProtectFile(file.wpath(), newName, rights, watermark, expire, tagstring, userMemberId)) != 0)
		{
			return NxlPath();
		}
		return NxlPath(newName);
	}
	RPMSession::RPMFolderStatus FolderStatus(const unsigned int status) {
		if (status == 0) {
			return RPMSession::NonRPMDir;
		}
		if (status & RPM_SAFEDIRRELATION_SAFE_DIR) {
			return RPMSession::IsRPMDir;
		}
		if (status & RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR) {
			return RPMSession::PosterityOfRPMDir;
		}
		if (status & RPM_SAFEDIRRELATION_ANCESTOR_OF_SAFE_DIR) {
			return RPMSession::AncestorOfRPMDir;
		}
		return RPMSession::UnknownStatus;
	}
	RPMSession::RPMFolderStatus rpmdir_status(const NxlPath &folder)
	{
		unsigned int status;
		SDRmRPMFolderOption option;
		std::wstring tags;
		if (RPM_CALL(_rmcInstance->IsRPMFolder(folder.wpath(), &status, &option, tags)) != 0)
		{
			return RPMSession::UnknownStatus;
		}
		DBGLOG("'%s':%u(%s)", folder.tstr(), status, RPMDirRelations_tostring(status).c_str());
		return FolderStatus(status);
	}
	bool rpmdir_remove(const NxlPath &folder, bool force = true)
	{
		DBGLOG("RemoveRPMDir('%s', force=%d)...", folder.tstr(), force);
		return RPM_CALL(_rmcInstance->RemoveRPMDir(folder.wpath(), force)) == 0;
	}
	void dir_list_files(const NxlPath &folder, bool bSubFolders)
	{
		if (!folder.IsValid()) return;
		std::vector<NxlPath> folders;
		folders.push_back(folder);
		for (int i = 0; i < folders.size();i++) {
			NxlPath sub = folders.at(i);
			const NxlPath &pattern = sub.AppendPath(L"*");
			DBGLOG("Enumerating-'%s'...", pattern.tstr());

			WIN32_FIND_DATAW ffd;
			HANDLE hFind = FindFirstFileW(pattern.wstr(), &ffd);
			if (INVALID_HANDLE_VALUE == hFind)
				return;

			do
			{
				NxlPath item = sub.AppendPath(ffd.cFileName);
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (ffd.cFileName[0] != '.')
					{
						if (bSubFolders)
						{
							folders.push_back(item);
							sub = folders.at(i);//update 
						}
						else
						{
							DBGLOG("<DIR> %s", item.tstr());
						}
					}
				}
				else
				{
					DBGLOG("%s", item.tstr());
				}
			} while (FindNextFile(hFind, &ffd) != 0);
			FindClose(hFind);
		}
	}
	bool rpmdir_add(const NxlPath &folder)
	{
		std::vector<std::pair<std::wstring, std::wstring>> renamedFiles;
		DBGLOG("AddRPMDir('%s')...", folder.tstr());
		//if (RPM_CALL(_rmcInstance->AddRPMDir(folder.wpath(), FCRAutoRenameNxlFile, &renamedFiles)) != 0)
		if (RPM_CALL(_rmcInstance->AddRPMDir(folder.wpath(), (SDRmRPMFolderOption::RPMFOLDER_NORMAL | SDRmRPMFolderOption::RPMFOLDER_OVERWRITE))) != 0)
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

	std::vector<SDR_OBLIGATION_INFO> user_eval_config_obligations(const std::wstring &obligationName, const wchar_t *resourceType=L"nxcadrmx") 
	{
#define RIGHT_CONFIGURE RIGHT_CLASSIFY
		std::vector<SDR_OBLIGATION_INFO> obligations;
		//user_list_bundle();{
		wchar_t buffer[MAX_PATH];
		GetModuleFileName(NULL, buffer, MAX_PATH);
		std::wstring resName = buffer;//use process name as resource name
		std::wstring resType = resourceType;//need match the policy model name
		std::vector<std::pair<std::wstring, std::wstring>> attrs;
		attrs.push_back(std::make_pair(L"configuration", L"TeamcenterRMX"));//TODO:
		std::vector<std::pair<SDRmFileRight, std::vector<SDR_OBLIGATION_INFO>>> rightsAndObligations;
		DBGLOG("GetResourceRightsFromCentralPolicies('%s', '%s')...", resName.c_str(), resType.c_str());
		if (RPM_CALL(this->_puser->GetResourceRightsFromCentralPolicies(resName, resType, attrs, rightsAndObligations)) == 0)
		{
			DBGLOG("Found %d Rights", rightsAndObligations.size());
			for (auto r : rightsAndObligations) {
				for (auto o : r.second)
				{
					DBGLOG("Obligation-%s(Right='%s')", o.name.c_str(), FileRight_tostring(r.first).c_str());
					if (o.name == obligationName
						&& r.first == RIGHT_CONFIGURE)
					{
						obligations.push_back(o);
					}
				}
			}
		}
		return obligations;
	}
	NxlMetadataPriorities user_get_metadata_priorities()
	{
		NxlMetadataPriorities priorities;
#define OBLIGATION_NAME L"TCDRM_TAGCONFIG"
#define TAGCONFIG_KEY L"TagKey"
#define TAGCONFIG_VALUES L"ValueList"
		for (auto o : user_eval_config_obligations(OBLIGATION_NAME)) {
				std::string key;
				std::vector<std::string> values;
				for (auto kvp : o.options) {
					DBGLOG("%s=%s", kvp.first.c_str(), kvp.second.c_str());
					if (kvp.first == TAGCONFIG_KEY)
					{
						key = wstring_to_utf8(kvp.second);
					}
					else if (kvp.first == TAGCONFIG_VALUES) {
						std::string valStr = wstring_to_utf8(kvp.second);
						for (int i = -1, j = 0; j <= valStr.length();j++) {
							if (j == valStr.length() || valStr[j] == ';')
							{
								//add sub str
								if (j - i > 1) {
									values.push_back(valStr.substr(i + 1, j - i - 1));
								}
								i = j;
							}
						}
					}
				}
				if (!key.empty() && !values.empty()) {
					//TODO:what if duplicated?
					priorities.insert(std::make_pair(key, values));
				}
		}
		return priorities;
	}
	void user_add_activity_log(const NxlPath &inputFile, const std::string &right, bool isAllowed) {
		RM_ActivityLogOperation op;
		if (right == "RIGHT_EDIT")
		{
			op = RL_OEdit;
		}
		else if (right == "RIGHT_VIEW") {
			op = RL_OView;
		}
		else if (right == "RIGHT_DOWNLOAD") {
			op = RL_ODownload;
		}
		else if (right == "RIGHT_PRINT") {
			op = RL_OPrint;
		}
		else
		{
			auto msg = std::string("unsupported right-") + right;
			SetError(ResultInfo(1, msg.c_str(), __FUNCTION__, __FILE__, __LINE__));
			return;
		}
		if (RPM_CALL(_puser->AddActivityLog(inputFile.wpath(), op, isAllowed ? RL_RAllowed : RL_RDenied)) == 0)
		{
			DBGLOG("AddActivityLog('%s', %S(%d), allowed=%d) success", inputFile.wstr(), right.c_str(), op, isAllowed);
		}
		else
		{
			ERRLOG("AddActivityLog('%s', %S(%d), allowed=%d) failed", inputFile.wstr(), right.c_str(), op, isAllowed);
		}
	}
	inline const ResultInfo &GetLastError() const{
		return _lastError;
	}
	inline const ResultInfo &SetError(const ResultInfo &&err=ResultInfo()) {
		_lastError = err;
		if (err.Code() != 0)
		{
			ERRLOG("[Code-%lu]%S(file-'%S' func-%S Line-%d)", err.Code(), err.Message(), err.file(), err.func(), err.line());
		}
		return _lastError;
	}
private:
	bool _initialized;
	ResultInfo _lastError;
	std::string _passcode;
	ISDRmTenant *_pTenant;
	ISDRmUser *_puser;
	ISDRmcInstance *_rmcInstance;
};
