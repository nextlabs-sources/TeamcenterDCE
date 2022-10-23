#include "SDRmSession.h"

#include <algorithm>
#include <map>

#include "CommonUtils.h"
#include "..\..\CADRMXCommon\RegKeyEntry.h"

#include <SDLAPI.h>

using namespace std;

// Hard-coded variables
static string LOGIN_PASSCODE = "{6829b159-b9bb-42fc-af19-4a6af3c9fcf6}";
#define REG_KEY_CREORMX L"Software\\NextLabs\\CreoRMX"
#define REG_VALUE_RPMDIR L"RPMDir"
#define REG_SAM_DESIRED  KEY_WRITE|KEY_QUERY_VALUE|KEY_WOW64_64KEY

static map<uint32_t, wchar_t*> s_rpmDirRelations = {
	{ RPM_SAFEDIRRELATION_SAFE_DIR, L"Safe dir" },
	{ RPM_SAFEDIRRELATION_ANCESTOR_OF_SAFE_DIR, L"Ancestor of safe dir" },
	{ RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR, L"Posterity of safe dir" },
	{ 0, L"Not safe dir" }
};

inline bool _IsRPMDir(DWORD sdrDirStatus)
{
	return (sdrDirStatus & RPM_SAFEDIRRELATION_SAFE_DIR || sdrDirStatus & RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR);
}

inline void _AppendDir(wstring& strRoot, const wstring& strSubDir)
{
	const wchar_t lastChar = strRoot.back();
	if (lastChar != L'\\' && lastChar != L'/')
		strRoot += L'\\';
	strRoot += strSubDir;
}

inline wstring _GetSafeDirName(DWORD sdrDirStatus)
{
	for (auto& dirRel : s_rpmDirRelations)
	{
		if (dirRel.first & sdrDirStatus)
		{
			return wstring(dirRel.second);
		}
	}

	return s_rpmDirRelations[0];
}

CSDRmSession::CSDRmSession()
	: m_pDRmcInstance(nullptr)
	, m_pDRmUser(nullptr)
	, m_bInit(false)
{	
	string passcode(LOGIN_PASSCODE);
	ISDRmTenant* pDRmTenant;
	SDWLResult result = RPMGetCurrentLoggedInUser(passcode, m_pDRmcInstance, pDRmTenant, m_pDRmUser);

	if (!result || m_pDRmcInstance == nullptr || m_pDRmUser == nullptr)
	{
		LOG_INFO(L"[ERROR] Cannot obtain the login information (error:%s)", result.ToString().c_str());
	}
	else
	{
		m_bInit = true;
	}
}

CSDRmSession::~CSDRmSession()
{
	if (m_bInit)
	{
		SDWLibDeleteRmcInstance(m_pDRmcInstance);
		m_bInit = false;
	}
	m_pDRmcInstance = nullptr;
	m_pDRmUser = nullptr;	
}

bool CSDRmSession::RPMRegisterApp(const wstring& appPath)
{
	if (!m_bInit)
		return false;

	bool alreadyRegistered;
	if (m_pDRmcInstance->RPMIsAppRegistered(appPath, alreadyRegistered) && alreadyRegistered)
	{
		LOG_INFO(L"RMP app already registered: %s", appPath.c_str());
		return true;
	}

	SDWLResult ret = m_pDRmcInstance->RPMRegisterApp(appPath, LOGIN_PASSCODE);
	if (!ret)
	{
		LOG_INFO(L"[ERROR] Cannot register app: '%s' (error:%s)", appPath.c_str(), ret.ToString().c_str());
		return false;
	}
	return true;
}

bool CSDRmSession::RPMUnregisterApp(const wstring & appPath)
{
	if (!m_bInit)
		return false;

	SDWLResult ret = m_pDRmcInstance->RPMUnregisterApp(appPath, LOGIN_PASSCODE);
	if (!ret)
	{
		LOG_INFO(L"[ERROR] Cannot unregister app: '%s' (error:%s)", appPath.c_str(), ret.ToString().c_str());
		return false;
	}
	return true;
}

bool CSDRmSession::RPMIsAppRegistered(const wstring& appPath, bool& isRegistered) const
{
	if (!m_bInit)
		return false;

	SDWLResult ret = m_pDRmcInstance->RPMIsAppRegistered(appPath, isRegistered);
	if (!ret)
	{
		LOG_INFO(L"[ERROR] Cannot check if app is registered (error:%s)", ret.ToString().c_str());
		return false;
	}
	return true;
}

bool CSDRmSession::IsRPMDir(const wstring& dirPath, bool& isRPMDir) const
{
	if (!m_bInit)
		return false;

	uint32_t dirStatus;
	SDRmRPMFolderOption option;
	std::wstring fileTags;
	SDWLResult result = m_pDRmcInstance->IsRPMFolder(dirPath, &dirStatus, &option, fileTags);
	if (!result)
	{
		LOG_INFO(L"[ERROR] Cannot check if RPM dir '%s '(error:%s)", dirPath.c_str(), result.ToString().c_str());
		return false;
	}
	isRPMDir = _IsRPMDir(dirStatus);
	LOG_INFO(L"%s (Relation %d: '%s')", isRPMDir ? L"yes" : L"no", dirStatus, _GetSafeDirName(dirStatus).c_str());
	
	return true;
}

bool CSDRmSession::AddRPMDir(const wstring& dirPath)
{
	if (!m_bInit)
		return false;

	// since 5.4: Apply RPMFOLDER_OVERWRITE | RPMFOLDER_API options
	SDWLResult result = m_pDRmcInstance->AddRPMDir(dirPath, RPMFOLDER_OVERWRITE | RPMFOLDER_API);
	if (!result)
	{
		LOG_INFO(L"[ERROR] Cannot add RPM dir '%s '(error:%s)", dirPath.c_str(), result.ToString().c_str());
		return false;
	}
	return true;
}

bool CSDRmSession::RemoveRPMDir(const wstring& dirPath)
{
	if (!m_bInit)
		return false;

	SDWLResult result = m_pDRmcInstance->RemoveRPMDir(dirPath, true);
	if (!result)
	{
		LOG_INFO(L"[ERROR] Cannot remove RPM dir '%s' (error:%s)", dirPath.c_str(), result.ToString().c_str());
		return false;
	}
	
	// Also clear up the cached value in registry in case rpm folder will
	// be added when launching Creo RMX.
	CRegKeyEntry regKey;
	vector<wstring> regValues;
	LONG ret = regKey.Open(HKEY_CURRENT_USER, REG_KEY_CREORMX, REG_SAM_DESIRED);
	if (REG_SUCCEEDED(ret))
	{
		ret = regKey.QueryMultiStringValue(REG_VALUE_RPMDIR, regValues);
		if (REG_SUCCEEDED(ret))
		{
			auto foundIter = std::find(regValues.begin(), regValues.end(), dirPath);
			if (foundIter != regValues.end())
			{
				regValues.erase(foundIter);
				ret = regKey.SetMultiStringValue(REG_VALUE_RPMDIR, regValues);
			}
		}
		regKey.Close();
	}
	return true;
}

bool CSDRmSession::GetAllRPMDirs(const wstring& rootDir, vector<wstring>& retDirs) const
{
	if (!m_bInit)
		return false;
	
	WIN32_FIND_DATA findData;
	wstring searchDir(rootDir);
	_AppendDir(searchDir, L"*");
	HANDLE hFind = FindFirstFile(searchDir.c_str(), &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		LOG_INFO(L"[ERROR] Cannot find dir '%s'", rootDir.c_str());
		return false;
	}
	do
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && findData.cFileName[0] != L'.'
			&& wcsncmp(findData.cFileName, L"..", 2) != 0)
		{
			 wstring dirPath(rootDir);
			_AppendDir(dirPath, findData.cFileName);

			uint32_t dirStatus;
			SDRmRPMFolderOption option;
			std::wstring fileTags;
			SDWLResult result = m_pDRmcInstance->IsRPMFolder(dirPath, &dirStatus, &option, fileTags);
			if (!result)
			{
				LOG_INFO(L"[ERROR] Cannot check if RPM dir '%s '(error:%s)", dirPath.c_str(), result.ToString().c_str());
			}
			else if (_IsRPMDir(dirStatus))
			{
				retDirs.push_back(dirPath);
				LOG_INFO(L"%s (Relation %d: '%s')", dirPath.c_str(), dirStatus, _GetSafeDirName(dirStatus).c_str());
			}
		}
	} while (FindNextFile(hFind, &findData));
	DWORD dwErr = GetLastError();
	if (dwErr != ERROR_NO_MORE_FILES)
	{
		LOG_INFO(L"[ERROR] FindNextFile died for some reason: %lu", dwErr);
	}
	FindClose(hFind);
	return true;
}

bool CSDRmSession::GetPolicyBundle(string& retPolicies) const
{
	if (!m_bInit)
		return false;

	string defaultTenant = m_pDRmUser->GetDefaultTokenGroupName();
	if (!defaultTenant.empty())
	{
		//m_pDRmUser->GetPolicyBundle(Utils::s2ws(defaultTenant), retPolicies);
		if (!retPolicies.empty())
		{
			cout << retPolicies.c_str() << endl;
		}
	}

	return true;
}

wstring CSDRmSession::GetSDKVersion()
{
	DWORD dwVer = SDWLibGetVersion();
	DWORD dwMajorVer = (DWORD)(HIBYTE(HIWORD(dwVer)));
	DWORD dwMinorVer = (DWORD)(LOBYTE(HIWORD(dwVer)));
	DWORD dwBuild = (DWORD)(LOWORD(dwVer));
	wstring strVer = std::to_wstring(dwMajorVer) + wstring(L".") +
						std::to_wstring(dwMinorVer) + wstring(L".") +
						std::to_wstring(dwBuild);

	wcout << strVer.c_str() << endl;
	return strVer;
}
