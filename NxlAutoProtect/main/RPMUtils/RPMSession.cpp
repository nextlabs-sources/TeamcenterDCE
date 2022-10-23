// RPMUtils.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "RPMUtils.h"
#include <internals/RPMSessionImpl.h>
#include <NxlUtils/include/utils_rmc.h>

#include <CADRMXCommon/RMXTagHlp.h>
#include <CADRMXCommon/RMXViewOverlay.h>
#include <CADRMXCommon/RMXUtils.h>

#define MAKE_ERROR(code, msg)   ResultInfo(code, msg, __FUNCTION__, __FILE__, __LINE__)

#define ERROR_INVALID_ARG(msg) MAKE_ERROR(1, msg)
#define ERROR_WINDOWS(wincall) GetWinErr(__FUNCTION__ ":" wincall, __FILE__, __LINE__)
#define MAKE_ERROR_FILE_NOT_FOUND(file)  MAKE_ERROR(ERROR_FILE_NOT_FOUND, (std::string("FileNotFound:")+ file).c_str())
#define MAKE_ERROR_FILE_IS_PROTECTED(file)  MAKE_ERROR(ERROR_FILE_IS_PROTECTED, (std::string("FileIsProtected:")+ file).c_str())


#define IMPL_RPM_CALL(x) (_impl->_lastError = report_rpm_error(x, __FUNCTION__, __FILE__, __LINE__, #x))

// This is the constructor of a class that has been exported.
// see RPMUtils.h for the class definition
RPMSession::RPMSession(const char *passcode)
{
	this->_impl = new RPMSessionImpl(passcode);
    return;
}
RPMSession::~RPMSession()
{
	if (this->_impl != nullptr)
	{
		delete this->_impl;
	}
	this->_impl = nullptr;
}
const ResultInfo &RPMSession::GetLastError()
{
	return _impl->GetLastError();
}
bool RPMSession::IsRPMLoggedIn()
{
	return _impl->Initialize(true);
}
const ResultInfo & RPMSession::SetTrustedProcess(DWORD pid, bool isAdd)
{
	// TODO: insert return statement here
	if (!_impl->Initialize())
		return _impl->GetLastError();
	_impl->set_trusted_process(pid, isAdd);
	return _impl->GetLastError();
}
const ResultInfo & RPMSession::RegisterApp(bool isRegister, const wchar_t * app)
{
	if (!_impl->Initialize())
		return _impl->GetLastError();

		//register application
	if (app == nullptr || wcslen(app) == 0) {
		wchar_t buffer[MAX_PATH];
		GetModuleFileName(NULL, buffer, MAX_PATH);
		_impl->register_application(isRegister, NxlPath(buffer));
	}
	else
	{
		_impl->register_application(isRegister, NxlPath(app));
	}
	return _impl->GetLastError();
}
const ResultInfo &RPMSession::SetRMXStatus(bool status)
{
	if (!_impl->Initialize())
		return _impl->GetLastError();

	if (status)
	{
		if (RegisterApp(true) != 0)
		{
			return _impl->GetLastError();
		}
	}
	_impl->rmx_set_status(status);
	return _impl->GetLastError();
}
const ResultInfo & RPMSession::IsAppRegistered(const wchar_t * appPath, bool *oRegistered)
{
	if (appPath == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:appPath=null)"));
	if (oRegistered == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:oRegistered=null)"));
	if (!_impl->Initialize())
		return _impl->GetLastError();
	const NxlPath path(appPath);
	if (!path.CheckFileExists())
		return _impl->SetError(MAKE_ERROR_FILE_NOT_FOUND(path.str()));

	*oRegistered = _impl->is_application_registered(path);
	return _impl->GetLastError();
}
const ResultInfo &RPMSession::AddRPMFolder(const wchar_t *f, bool *pNewRPMFolder, bool refresh)
{
	if (f == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:folder=null)"));
	/*
	if (pNewRPMFolder == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:pNewRPMFolder=null"));
		*/
	if (!_impl->Initialize()) 
		return _impl->GetLastError();

	const NxlPath folder(f);
	RPMFolderStatus status;
	if ((status = _impl->rpmdir_status(folder)) == IsRPMDir)
	{
		//already safe dir
		if (pNewRPMFolder != nullptr)
			*pNewRPMFolder = false;
	}
	else
	{
		if (refresh)
		{
			//always add rpmdir in any other cases
			_impl->dir_list_files(folder, false);
		}
		if (_impl->rpmdir_add(folder))
		{
			if (pNewRPMFolder != nullptr)
				*pNewRPMFolder = true;
			if (refresh && !FolderStatus_IsInRpm(status))
			{
				//workaround fix for RPM:use FindFirstFile to notify RPM that the directory has been set as RPM folder
				_impl->dir_list_files(folder, true);
			}
		}
	}
	return _impl->GetLastError();
}
const ResultInfo &RPMSession::RemoveRPMFolder(const wchar_t *folder, bool force)
{
	if (folder == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:folder=null)"));
	if (!_impl->Initialize())
		return _impl->GetLastError();

	const NxlPath folderPath(folder);
	//TODO:check rpmdir_status?
	_impl->rpmdir_remove(folderPath, force);
	return _impl->GetLastError();
}
const ResultInfo &RPMSession::RPMEditCopy(const wchar_t *file, wchar_t rpmDir[MAX_PATH])
{
	if (file == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:file=null)"));
	//TODO:check if file is protected?
	if (!_impl->Initialize())
		return _impl->GetLastError();

	const NxlPath filePath = NxlPath::NormalizeFilePath(file);
	if (!filePath.CheckFileExists())
		return _impl->SetError(MAKE_ERROR_FILE_NOT_FOUND(filePath.str()));

	//TODO:check if input directory is rpm dir?
	//TODO:check if input file is protected?
	tstring newPath(rpmDir);
	DBGLOG("Copying '%s' to '%s'", filePath.tstr(), newPath.c_str());
	if (IMPL_RPM_CALL(_impl->_rmcInstance->RPMEditCopyFile(filePath.wstr(), newPath)) == 0)
	{
		NxlPath rpmFile(newPath);
		DBGLOG("NewRPMFile='%s'", rpmFile.tstr());
		//WORKAROUND:RPM bug, when the input file doesn't have .nxl extension
		//1, the returned file path doesn't have file extension
		//2, the file in intemediate folder doesn't have .nxl extension
		const wchar_t *oldext = filePath.FindExtension();
		if (oldext != nullptr
			&& wcsicmp(oldext, L".nxl") != 0)//input doesn't have .nxl extension
		{
			const wchar_t *newext = rpmFile.FindExtension();
			//new extension doesn't match old extension
			if (newext == nullptr || wcsicmp(newext, oldext) != 0)
			{
				//append old extension
				auto addext = rpmFile.Append(oldext);
				if (ASSERT_FILE_EXISTS(addext))
				{
					DBGLOG("FileExists:'%s'", addext.tstr());
					rpmFile = addext;
				}
			}
		}
		if (!rpmFile.HasNxlExtension())
		{
			//move file
			NxlPath nxlfile = rpmFile.Append(".nxl");
			if (!ASSERT_FILE_EXISTS(nxlfile))
			{
				if (!MoveFileW(rpmFile.wstr(), nxlfile.wstr()))
				{
					_impl->SetError(ERROR_WINDOWS("CopyFile"));
				}
				else
				{
					DBGLOG("Renamed '%s'", nxlfile.tstr());
				}
			}
			//
		}
		//return
		wcscpy_s(rpmDir, MAX_PATH, rpmFile.wstr());
		//clear;
		_impl->SetError();
	}
	return _impl->GetLastError();
}
const ResultInfo &RPMSession::RPMEditSave(SaveMode mode, const wchar_t *rpmfile, const wchar_t *nonrpm)
{
	if (rpmfile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:rpmfile=null)"));
	if (nonrpm == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:nonrpm=null)"));

	if (!_impl->Initialize())
		return _impl->GetLastError();
	NxlPath rpmpath = NxlPath::NormalizeFilePath(rpmfile);
	NxlPath nonpath(nonrpm);
	ASSERT_FILE_EXISTS(rpmpath);//warning
	bool deleterpm = true;
	int saveexit = 0;
	switch (mode) {
	case FinishWithoutSave:
		DBGLOG("Deleting '%s'...", rpmpath.tstr());
		deleterpm = true;
		saveexit = 2;//exit and no save
		break;
	case SaveOnly:
		DBGLOG("Saving '%s' to '%s'...", rpmpath.tstr(), nonpath.tstr());
		ASSERT_FILE_EXISTS(nonpath);//warning
		deleterpm = true;
		saveexit = 1;
		break;
	case SaveAndFinish:
	default:
		DBGLOG("FinalSaving '%s' to '%s'...", rpmpath.tstr(), nonpath.tstr());
		ASSERT_FILE_EXISTS(nonpath);//warning
		deleterpm = true;
		saveexit = 3;
		break;
	}
	if (IMPL_RPM_CALL(_impl->_rmcInstance->RPMEditSaveFile(rpmpath.wpath(), nonpath.wpath(), deleterpm, saveexit)) == 0)
	{
		if (nonpath.IsValid())
		{
			DBGLOG("Saved='%s'", nonpath.tstr());
		}
		else
		{
			DBGLOG("Deleted='%s'", rpmpath.tstr());
		}
	}
	return _impl->GetLastError();
}

const ResultInfo & RPMSession::RPMDeleteFile(const wchar_t * nxlfile)
{
	if (nxlfile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:file=null)"));
	if (!_impl->Initialize())
		return _impl->GetLastError();
	const NxlPath filePath = NxlPath::NormalizeFilePath(nxlfile);
	DBGLOG("Deleting...'%s'", filePath.tstr());
	return IMPL_RPM_CALL(_impl->_rmcInstance->RPMDeleteFile(nxlfile));
}

const ResultInfo &RPMSession::IsFileInRPMDir(const wchar_t *file, bool *isinrpm)
{
	if (file == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:file=null)"));
	if (isinrpm == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:isinrpm=null)"));
	if (!_impl->Initialize())
		return _impl->GetLastError();
	const NxlPath filePath = NxlPath::NormalizeFilePath(file);
	const NxlPath folderPath = filePath.RemoveFileSpec();
	*isinrpm = FolderStatus_IsInRpm(_impl->rpmdir_status(folderPath));
	return _impl->GetLastError();
}

const ResultInfo &RPMSession::CheckFolderStatus(const wchar_t *folder, RPMFolderStatus *oStatus)
{
	if (folder == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:folder=null)"));
	if (oStatus == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:oStatus=null)"));
	if (!_impl->Initialize())
		return _impl->GetLastError();
	const NxlPath folderPath(folder);
	*oStatus = _impl->rpmdir_status(folderPath);
	return _impl->GetLastError();
}
const ResultInfo &RPMSession::ProtectFile(const wchar_t *inputFile, const NxlMetadataCollection &outputTags, bool forceNxlExt)
{
	if (inputFile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:inputFile=null)"));
	if (!_impl->Initialize())
		return _impl->GetLastError();
	const NxlPath filePath = NxlPath::NormalizeFilePath(inputFile);
	if (!filePath.CheckFileExists())
		return _impl->SetError(MAKE_ERROR_FILE_NOT_FOUND(filePath.str()));

	auto folderPath = filePath.RemoveFileSpec();
	bool isRpmDir = FolderStatus_IsInRpm(_impl->rpmdir_status(folderPath));
	if (_impl->user_is_file_protected(filePath)) {
		//if in rpm folder, check if the curent process has read right on this file
		if (!isRpmDir || file_is_protected(filePath.str()))
		{
			return _impl->SetError(MAKE_ERROR_FILE_IS_PROTECTED(filePath.str()));
		}
		//TODO:let caller decide if the reprotect is required
		DBGLOG("The protected file resides in RPM folder and the current process has read right, we need reprotect this file for consisitence");
	}
	//reduce metadata collection with priorities
	//auto metadatapriorities = _impl->user_get_metadata_priorities();
	//auto collectionImpl = outputTags.GetImpl()->ReduceWithPriorities(metadatapriorities);
	std::string inputTags = outputTags.str();
	std::string mergedTags;
	RMXLib::MergeTags(_impl->_puser, inputTags, mergedTags);
	if (inputTags.length() != mergedTags.length()) {
		DBGLOG("InputTagString(%d)='%S'", inputTags.length(), inputTags.c_str());
	}

	//protect file
	DBGLOG("ProtectFile('%s', (%d)'%S')...", filePath.tstr(), mergedTags.length(), mergedTags.c_str());
	const NxlPath protectedFile = _impl->user_protect_file(filePath, mergedTags.c_str());
	if(!protectedFile.IsValid())
		return _impl->GetLastError();
	DBGLOG("Protected:'%s'", protectedFile.tstr());

	//delete old file
	if (isRpmDir)
	{
		DBGLOG("RPMDeleteFile(%s)...", filePath.tstr());
		IMPL_RPM_CALL(_impl->_rmcInstance->RPMDeleteFile(filePath.wpath()));
	}
	if (filePath.CheckExist()) {
		DBGLOG("Delete original file...'%s'", filePath.tstr());
		if (!DeleteFileW(filePath.wstr()))
		{
			_impl->SetError(ERROR_WINDOWS("DeleteFile"));
		}
	}

	ResultInfo retError;
	const NxlPath &tarFile = (forceNxlExt || isRpmDir) ? filePath.EnsureNxlExtension() : filePath;
	//TODO:when the file is in non-rpm folder, we should check whether there is another tarFile existed in this folder
	DBGLOG("Copying protected file to '%s'", tarFile.tstr());
	if (!CopyFileW(protectedFile.wstr(), tarFile.wstr(), false))
	{
		retError = _impl->SetError(ERROR_WINDOWS("CopyFile"));
	}
	//remove protected file from RPM file manager;
	DBGLOG("Removing Protected file...'%s'", protectedFile.tstr());
	DeleteFileW(protectedFile.wstr());
	//WORKAROUND:when new .prt.nxl is created in rpm dir, call file status to notify rpm
	if (!filePath.HasNxlExtension()
		&& tarFile.HasNxlExtension()
		&& isRpmDir)
	{
		bool isprotected;
		_impl->file_status(tarFile, &isprotected);
	}
	return _impl->_lastError = retError;//return CopyFile error only
}
const ResultInfo &RPMSession::ReadFileTags(const wchar_t *inputFile, bool *oIsProtected, NxlMetadataCollection &outputTags)
{
	if (inputFile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:inputFile=null)"));

	const NxlPath filePath(inputFile);
	if (!filePath.CheckFileExists())
		return _impl->SetError(MAKE_ERROR_FILE_NOT_FOUND(filePath.str()));

	if (!_impl->Initialize())
		return _impl->GetLastError();

	bool isprotected;
	RPMFolderStatus folderStatus;
	if (_impl->file_status(filePath, &isprotected, &folderStatus))
	{
		if (oIsProtected != nullptr)
			*oIsProtected = isprotected;
		if (isprotected)
		{
			if (FolderStatus_IsInRpm(folderStatus))
			{
				//in rpm folder, user helper
				_impl->helper_read_metadata(filePath, outputTags);
			}
			else
			{
				_impl->user_read_metadata(filePath, outputTags);
			}
		}
	}
	return _impl->GetLastError();
}
const ResultInfo &RPMSession::RPMReadFileTags(const wchar_t *inputFile, bool *oIsProtected, NxlMetadataCollection &outputTags)
{
	if (inputFile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:inputFile=null)"));

	const NxlPath filePath = NxlPath::NormalizeFilePath(inputFile);
	if (!filePath.CheckFileExists())
		return _impl->SetError(MAKE_ERROR_FILE_NOT_FOUND(filePath.str()));

	if (!_impl->Initialize())
		return _impl->GetLastError();

	bool isprotected = _impl->user_is_file_protected(filePath);
	if (oIsProtected != nullptr)
		*oIsProtected = isprotected;

	_impl->user_read_metadata(filePath, outputTags);

	return _impl->GetLastError();
}
const ResultInfo &RPMSession::HelperReadFileTags(const wchar_t *inputFile, bool *oIsProtected, NxlMetadataCollection &outputTags)
{
	if (inputFile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:inputFile=null)"));

	const NxlPath filePath = NxlPath::NormalizeFilePath(inputFile);
	if (!filePath.CheckFileExists())
		return _impl->SetError(MAKE_ERROR_FILE_NOT_FOUND(filePath.str()));

	//TODO:still need?
	if (!_impl->Initialize())
		return _impl->GetLastError();

	_impl->helper_read_metadata(filePath, outputTags, oIsProtected);
	return _impl->GetLastError();
}

const ResultInfo & RPMSession::HelperCheckFileProtection(const wchar_t * inputFile, bool * oIsProtected)
{
	if (inputFile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:inputFile=null)"));
	if (oIsProtected == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:oIsProtected=null)"));
	const NxlPath filePath = NxlPath::NormalizeFilePath(inputFile);
	if (!filePath.CheckFileExists())
		return _impl->SetError(MAKE_ERROR_FILE_NOT_FOUND(filePath.str()));

	//TODO:still need?
	if (!_impl->Initialize())
		return _impl->GetLastError();
	_impl->helper_is_file_protected(filePath, oIsProtected);
	return _impl->GetLastError();
}

const ResultInfo & RPMSession::ValidateFileExtension(const wchar_t * inputFile, bool *oIsProtected, bool *oInRpmDir)
{
	if (inputFile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:inputFile=null)"));
	/*if oIsProtected is null, don't output
	if (oIsProtected == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:oIsProtected=null)"));*/
	const NxlPath &filePath = NxlPath::NormalizeFilePath(inputFile);
	if (!filePath.CheckFileExists())
		return _impl->SetError(MAKE_ERROR_FILE_NOT_FOUND(filePath.str()));

	if (!_impl->Initialize())
		return _impl->GetLastError();

	bool isFileProtected;
	bool isInRpmDir = FolderStatus_IsInRpm(_impl->rpmdir_status(filePath.RemoveFileSpec()));
	if (oInRpmDir != nullptr) {
		*oInRpmDir = isInRpmDir;
	}
	if (_impl->helper_is_file_protected(filePath, &isFileProtected))
	{
		DBGLOG("'%s':IsFileProtected=%d IsInRpm=%d", filePath.tstr(), isFileProtected, isInRpmDir);
		if (oIsProtected != nullptr) {
			*oIsProtected = isFileProtected;
		}
		if (isFileProtected && isInRpmDir)
		{
			const NxlPath newName = filePath.EnsureNxlExtension();
			//in rpm folder, check .nxl extension
			if (!newName.CheckFileExists())
			{
				if (!MoveFileW(inputFile, newName.wstr()))
				{
					return _impl->SetError(ERROR_WINDOWS("MoveFile"));
				}
				DBGLOG("'%s':Renamed to '%s'", filePath.tstr(), newName.tstr());
			}
		}
	}
	return _impl->GetLastError();;
}

bool RPMSession::IsNxlProtectionEnabled()
{
	if (!_impl->Initialize())
		return true;
#define OB_TCDRM_SKIP_PROTECTION "TCDRM_SKIP_PROTECTION"
	auto obligations = _impl->user_eval_config_obligations(WTEXT(OB_TCDRM_SKIP_PROTECTION));
	INFOLOG("found %d obligations with name-%s", obligations.size(), TEXT(OB_TCDRM_SKIP_PROTECTION));
	return obligations.empty();
}

const ResultInfo & RPMSession::Logout()
{
	if (!_impl->Initialize())
		return _impl->GetLastError();
	DBGLOG("LogoutUser()...");
	return IMPL_RPM_CALL(_impl->_puser->LogoutUser());
}

const ResultInfo & RPMSession::ListPolicies()
{
	if (!_impl->Initialize())
		return _impl->GetLastError();
	
	auto policies = _impl->user_get_policies();
	for (auto p : policies) {
		INFOLOG("%s", p.c_str());
	}
	return _impl->GetLastError();
}

const ResultInfo & RPMSession::CheckFileRights(const wchar_t *inputFile, const char *right, bool &oIsAllowed, bool audit)
{
	if (inputFile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:inputFile=null)"));
	const NxlPath filePath = NxlPath::NormalizeFilePath(inputFile);
	if (!filePath.CheckFileExists())
		return _impl->SetError(MAKE_ERROR_FILE_NOT_FOUND(filePath.str()));

	if (!_impl->Initialize())
		return _impl->GetLastError();

	std::vector<tstring> rights;
	bool isprotected = _impl->user_is_file_protected(filePath);
	bool rightDefined = false;
	auto wright = utf8_to_wstring(right);
	if (isprotected) {
		if (_impl->rpm_get_rights(filePath, rights))
		{
			tostringstream ss;
			for (auto r : rights) {
				ss << r << TEXT(";");
				if (r == wright) {
					rightDefined = true;
				}
			}
			DBGLOG("Rights('%s'):%s", filePath.tstr(), ss.str().c_str());
			if (audit) {
				_impl->user_add_activity_log(filePath, right, rightDefined);
			}
			_impl->SetError();//clear error
		}
		else
		{
			DBGLOG("Rights('%s'):failed to get rights", filePath.tstr());
		}
	}
	else {
		DBGLOG("Rights('%s'):NotProtected", filePath.tstr());
	}
	//set output value
	oIsAllowed = !isprotected || rightDefined;
	return _impl->GetLastError();
}
const ResultInfo &RPMSession::GetFileRights(const wchar_t *inputFile, bool &oIsProtected, string_list_p oRights)
{
	if (inputFile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:inputFile=null)"));
	const NxlPath filePath = NxlPath::NormalizeFilePath(inputFile);
	if (!filePath.CheckFileExists())
		return _impl->SetError(MAKE_ERROR_FILE_NOT_FOUND(filePath.str()));

	if (!_impl->Initialize())
		return _impl->GetLastError();

	oIsProtected = _impl->user_is_file_protected(filePath);
	if (oIsProtected) {
	std::vector<tstring> rights;
		if (_impl->rpm_get_rights(filePath, rights))
		{
			tostringstream ss;
			for (auto r : rights) {
				ss << r << TEXT(";");
				if (oRights != NULL)
				{
					string_list_add(oRights, wstring_to_utf8(r).c_str());
				}
			}
			DBGLOG("Rights('%s'):%s", filePath.tstr(), ss.str().c_str());
			_impl->SetError();//clear error
		}
		else
		{
			DBGLOG("Rights('%s'):failed to get rights", filePath.tstr());
		}
	}
	else {
		DBGLOG("Rights('%s'):NotProtected", filePath.tstr());
	}
	return _impl->GetLastError();

}

const ResultInfo & RPMSession::AddActivityLog(const wchar_t *inputFile, const char *right, bool isAllowed)
{
	if (inputFile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:inputFile=null)"));
	const NxlPath filePath = NxlPath::NormalizeFilePath(inputFile);

	if (!_impl->Initialize())
		return _impl->GetLastError();
	_impl->user_add_activity_log(filePath, right, isAllowed);
	return _impl->GetLastError();
}

void RPMSession::ListRPMInfo()
{
	if (!_impl->Initialize())
		return;
	//list information
	bool login;
	std::wstring router, tenant, workingdir, tmpdir, sdklibdir;
	_impl->_rmcInstance->RPMGetCurrentUserInfo(router, tenant, workingdir, tmpdir, sdklibdir, login);
	INFOLOG("Router='%s'", router.c_str());
	INFOLOG("Tenant='%s'", tenant.c_str());
	INFOLOG("WorkingDir='%s'", workingdir.c_str());
	INFOLOG("TmpDir='%s'", tmpdir.c_str());
	INFOLOG("SdkLibDir='%s'", sdklibdir.c_str());
	INFOLOG("Tenant.Name='%s'", _impl->_pTenant->GetTenant().c_str());
	INFOLOG("Tenant.RMSURL='%s'", _impl->_pTenant->GetRMSURL().c_str());
	INFOLOG("Tenant.RouterURL='%s'", _impl->_pTenant->GetRouterURL().c_str());
	INFOLOG("User.DefaultTenantId='%S'", _impl->_puser->GetDefaultTenantId().c_str());
	INFOLOG("User.Name='%s'", _impl->_puser->GetName().c_str());
	bool adhoc;
	bool workspace = false;
	int heartbeat;
	int sysProjectId;
	std::string sysProjectTenantId, tenantId;
	if (_impl->_puser->GetTenantPreference(&adhoc, &workspace, &heartbeat, &sysProjectId, sysProjectTenantId, tenantId) == 0)
	{
		INFOLOG("TenantPreference:adhoc=%d heartbeat=%d sysProjectId=%d sysProjectTenantId='%S' tenantId='%S'", adhoc, heartbeat, sysProjectId, sysProjectTenantId.c_str(), tenantId.c_str());
		std::string midSysProj = _impl->_puser->GetMembershipID(sysProjectId);
		INFOLOG("GetMembershipID(sysProjectId)='%S'", midSysProj.c_str());
		std::string midSysProjTenant = _impl->_puser->GetMembershipID(sysProjectTenantId);
		INFOLOG("GetMembershipID(sysProjectTenantId)='%S'", midSysProjTenant.c_str());
	}
}

const ResultInfo &RPMSession::CheckFileProtection(const wchar_t *inputFile, bool *oIsProtected)
{
	if (inputFile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:inputFile=null)"));
	if (oIsProtected == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:oIsProtected=null)"));
	const NxlPath filePath = NxlPath::NormalizeFilePath(inputFile);
	if (!filePath.CheckFileExists())
		return _impl->SetError(MAKE_ERROR_FILE_NOT_FOUND(filePath.str()));

	if (!_impl->Initialize())
		return _impl->GetLastError();
	*oIsProtected = _impl->user_is_file_protected(filePath);
	return _impl->GetLastError();
}

const ResultInfo & RPMSession::CheckFileStatus(const wchar_t * inputFile, bool * oIsProtected, RPMFolderStatus * oFolderStatus)
{
	if (inputFile == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:inputFile=null)"));
	if (oIsProtected == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:oIsProtected=null)"));
	if (oFolderStatus == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:oFolderStatus=null)"));
	const NxlPath filePath(inputFile);
	if (!ASSERT_FILE_EXISTS(filePath))
		return _impl->SetError(MAKE_ERROR_FILE_NOT_FOUND(filePath.str()));

	if (!_impl->Initialize())
		return _impl->GetLastError();
	_impl->file_status(filePath, oIsProtected, oFolderStatus);
	return _impl->GetLastError();;
}

const ResultInfo &RPMSession::UpdateSecureOverlay(kvl_ro rmxAttrs,	string_list_ro contextFiles,	HWND targetWnd )
{
	if (targetWnd == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:targetWnd=null"));

	if (!_impl->Initialize())
		return _impl->GetLastError();

	if (!RMXLib::ClearViewOverlay(_impl->_rmcInstance, targetWnd))
	{
		//TODO:log error
		_impl->SetError(MAKE_ERROR(1,"RMXLib::ClearViewOverlay() failed"));
	}
	std::vector<std::wstring> files;
	if (contextFiles != nullptr)
	{
		for (unsigned int i = 0; i < contextFiles->count; i++)
		{
			NxlString path(contextFiles->items[i]);
			files.push_back(path.wstring());
			DBGLOG("File[%d/%d]'%s';", i + 1, contextFiles->count, files.back().c_str());
		}
	}
	if (files.size() > 0)
	{
		TEvalAttributeList ctxAttrs;
		if (rmxAttrs != nullptr)
		{
			for (unsigned int j = 0; j < rmxAttrs->count; j++)
			{
				auto key = rmxAttrs->keys[j];
				auto val = rmxAttrs->vals[j];
				ctxAttrs.push_back(TEvalAttribute(utf8_to_wstring(key), utf8_to_wstring(val)));
				DBGLOG("CtxAttrs[%d/%d]'%S'='%S'", j + 1, rmxAttrs->count, key, val);
			}
		}
		if (!RMXLib::SetViewOverlay(_impl->_rmcInstance, targetWnd, files, ctxAttrs))
		{
			return _impl->SetError(MAKE_ERROR(1, "RMXLib::SetViewOverlay() failed"));
		}
		DBGLOG("Window-%p:Overlay is updated", targetWnd);
	}
	else
	{
		DBGLOG("Window-%p:Overlay is cleared", targetWnd);
	}
	return _impl->GetLastError();
}
const ResultInfo& RPMSession::SetScreenCapture(HWND targetWnd, bool enable) {
	if (targetWnd == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:targetWnd=null"));
	RMXUtils::DisableScreenCapture(targetWnd, !enable);
	return _impl->GetLastError();
}
const ResultInfo& RPMSession::UpdateScreenCapture(HWND targetWnd, string_list_ro contextFiles) {
	if (targetWnd == nullptr)
		return _impl->SetError(ERROR_INVALID_ARG("invalid input argument:targetWnd=null"));
	std::vector<std::wstring> files;
	if (contextFiles != nullptr)
	{
		for (unsigned int i = 0; i < contextFiles->count; i++)
		{
			NxlString path(contextFiles->items[i]);
			files.push_back(path.wstring());
			DBGLOG("File[%d/%d]'%s';", i + 1, contextFiles->count, files.back().c_str());
		}
	}
	//disable screen capture 
	if (_impl->CheckRightForFiles(files, RIGHT_DECRYPT)) {
		DBGLOG("enable screen capture....");
		RMXUtils::DisableScreenCapture(targetWnd, false);
	}
	else
	{
		DBGLOG("disable screen capture....");
		RMXUtils::DisableScreenCapture(targetWnd, true);
	}
	return _impl->GetLastError();

}

void RPMSession::NotifyUser(const wchar_t *message, const wchar_t *context, const wchar_t *operation)
{
	if (!_impl->Initialize())
		return;

	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	IMPL_RPM_CALL(_impl->_rmcInstance->RPMNotifyMessage(PathFindFileNameW(buffer), context, message, 1, operation));	
}
