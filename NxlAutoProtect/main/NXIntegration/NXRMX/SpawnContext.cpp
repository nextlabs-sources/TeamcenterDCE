#include <NXRMX/SpawnContext.h>
#include <NXRMX/nx_utils.h>
#include <utils_windows.h>
#include <hook/hook_injector.h>
#include <NxlUtils/NxlTool.hpp>
#include <NxlUtils/RMXUtils.hpp>

SpawnContext SpawnContext::s_current;
SpawnContext SpawnContext::s_previous;
static NxlPath s_nxlhookDll;
static std::vector<NxlPath> s_nxlhookPaths;

const char *SpawnContext::OnAddingFileArgument(const char *file) {
	NxlPath filePath(file);
	/*if (filePath.HasNxlExtension())
	{
	//remove .nxl extension
	//currently only apply this fix on pdf files
	if (StringEndsWith(filePath.path(), ".pdf.nxl"))
	{
	//in this case, NX is overriding an existing protected pdf file via cgm2pdf
	//ext[0] = '\0';
	}
	}*/
	if (filePath.CheckFileExists())
	{
		bool isInRPMDir;
		if (g_rpmSession->IsFileInRPMDir(filePath.wstr(), &isInRPMDir) == 0
			&& !isInRPMDir)
		{
			bool isFileProtected;
			if (g_rpmSession->HelperCheckFileProtection(filePath.wstr(), &isFileProtected) == 0
				&& isFileProtected)
			{
				//if file is protected and folder is not rpm folder
				//we need use RPM EditCopy to create a copy in RPM folder
				wchar_t buffer[MAX_PATH] = L"";
				if (g_rpmSession->RPMEditCopy(filePath.wstr(), buffer) == 0)
				{
					NxlPath rpmfile(buffer);
					_fileMappings.push_back(std::make_pair(rpmfile, filePath));
					_fileArgs.push_back(std::move(rpmfile));
					return _fileArgs.back().str();
				}
			}
		}
	}
	_fileArgs.push_back(std::move(filePath));
	return file;
}
#define REG_KEY_RPM_FILES "SOFTWARE\\NextLabs\\NXRMX\\RPMFiles"
void SpawnContext::OnBeforeSpawning(const char *cmd) 
{
	//install CreateProcessHook
		//for cgm2pdf.exe, iges.exe, step203ug.exe, step214ug.exe, ugto2d.exe and dxfdwg.exe
	if (!s_nxlhookDll.IsValid()
		|| !ASSERT_FILE_EXISTS(s_nxlhookDll)) {
		if (s_nxlhookPaths.empty()) {
			s_nxlhookPaths.push_back(NxlPath(SysUtils::GetModuleDir()));
			s_nxlhookPaths.push_back(NxlPath(RMXUtils::getRmxRoot()).AppendPath(L"bin64"));
		}
		s_nxlhookDll = NxlTool::SearchExe(s_nxlhookPaths, L"MiniRMX" WTEXT(STRINGIFY(NX_MAJOR_VER)) L".dll");
	}
	inject_on_CreateProcess(s_nxlhookDll.str());
	_cmd = cmd;
	if (_fileMappings.size() <= 0)
		return;
	HKEY hkey;
	DWORD dwDisposition;
	if (REG_CALL(RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_RPM_FILES, 0, NULL, 0, KEY_WRITE, NULL, &hkey, &dwDisposition)))
	{
		for (auto iter = _fileMappings.begin(); iter != _fileMappings.end(); iter++) {
			//add the mapping to registery for extenal application access
			if (REG_CALL(RegSetValueEx(hkey, iter->first.str(), 0, REG_SZ, (PBYTE)iter->second.str(), (DWORD)iter->second.path().length())))
			{
				DBGLOG("ValueAdded:'%s'='%s'", iter->first.tstr(), iter->second.tstr());
			}
		}
		RegCloseKey(hkey);
	}
}
void SpawnContext::OnAfterSpawn(int pid) {
	_pid = pid;
	inject_on_CreateProcess(NULL);
	NXDBG("ProcessSpawned:PID=%d CommandName='%s' nFileArgs=%d nNxlFiles=%d", pid, _cmd.c_str(), _fileArgs.size(), _fileMappings.size());
	if (pid > 0)
	{
		extern DWORD _last_injected_pid;
		_injected = _last_injected_pid == pid;
		//update last spawned context and reset current;
		s_previous = *this;
	}
	if(_fileMappings.size() > 0
		&& !IsAlive())
	{
		//if process start failed or is not alive anymore, clean rpm files
		DBGLOG("Process-%lu is not alive", _pid);
		HKEY hkey;
		DWORD dwDisposition;
		if (REG_CALL(RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_RPM_FILES, 0, NULL, 0, KEY_WRITE, NULL, &hkey, &dwDisposition)))
		{
			for (auto iter = _fileMappings.begin(); iter != _fileMappings.end(); iter++) 
			{
				DBGLOG("Deleting...'%s'", iter->first.tstr());
				g_rpmSession->RPMEditSave(RPMSession::FinishWithoutSave, iter->first.wstr());
				REG_CALL(RegDeleteValue(hkey, iter->first.tstr()));
			}
			RegCloseKey(hkey);
		}

	}
	s_current = SpawnContext();//reset
}

bool SpawnContext::IsAlive() const
{
	if (_pid <= 0)
		return false;
	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, _pid);
	if (process == INVALID_HANDLE_VALUE) return false;
	DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	return ret == WAIT_TIMEOUT;
}
bool SpawnContext::wait_finish() const
{
	if (_pid > 0)
	{
		logical running;
		int status;
		while (NX_EVAL(UF_CFI_spawn_check_status(_pid, &running, &status)))
		{
			NXDBG("Process=%d Running=%d Status=%d", _pid, running, status);
			if (running)
			{
				//TODO:max wait time
				Sleep(500);
				continue;
			}
			return true;
		}
	}
	return false;
}
