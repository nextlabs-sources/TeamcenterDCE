#include "RMXInstance.h"

// System
#include <windows.h>

// Utils
#include "..\common\CreoRMXLogger.h"
#include <RMXUtils.h>
#include <SysErrorMessage.h>

// SkyDRM
#include <SDLAPI.h>

using namespace std;

namespace RPMHlp
{
	bool IsRPMDir(DWORD sdrDirStatus)
	{
		return (sdrDirStatus & RPM_SAFEDIRRELATION_SAFE_DIR || sdrDirStatus & RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR);
	}

	RMX_SAFEDIRRELATION GetRMXSafeDirType(DWORD sdrDirStatus)
	{
		RMX_SAFEDIRRELATION rmxSafeDirType;
		if (sdrDirStatus & RPM_SAFEDIRRELATION_SAFE_DIR)
			rmxSafeDirType = RMX_IsSafeDir;
		else if (sdrDirStatus & RPM_SAFEDIRRELATION_DESCENDANT_OF_SAFE_DIR)
			rmxSafeDirType = RMX_PosterityOfSafeDir;
		else if (sdrDirStatus & RPM_SAFEDIRRELATION_ANCESTOR_OF_SAFE_DIR)
			rmxSafeDirType = RMX_AncestorOfSafeDir;
		else
			rmxSafeDirType = RMX_NoneOfSafeDir;
		return rmxSafeDirType;
	}

} // namespace RPMHlp

CRMXInstance::CRMXInstance()
	: m_pDRmcInstance(nullptr)
	, m_pDRmUser(nullptr)
	, m_pDRmTenant(nullptr)
	, m_init(false)
{
}


CRMXInstance::~CRMXInstance()
{
	Terminate();
}

bool CRMXInstance::Initialize()
{
	if (m_init)
		return true;
	 
	CRMXLastResult result;

	string passcode(LOGIN_PASSCODE);
	result = RPMGetCurrentLoggedInUser(passcode, m_pDRmcInstance, m_pDRmTenant, m_pDRmUser);
	if (!result || m_pDRmcInstance == nullptr || m_pDRmUser == nullptr || m_pDRmTenant == nullptr)
	{
		CHK_ERROR_RETURN(true, false, L"Failed to obtain current logged in user from SkyDRM (error: %s)", result.ToString().c_str());
	}

	wchar_t szProcessName[MAX_PATH];
	if (GetModuleFileName(NULL, &szProcessName[0], MAX_PATH) == 0)
	{
		result = CSysErrorMessage(GetLastError());
		CHK_ERROR_RETURN(true, false, L"Cannot get the file name of current process (error: %s)", result.ToString().c_str());
	}
	
	bool alreadyRegistered = false;
	wstring appPath(szProcessName);
	if ((result = m_pDRmcInstance->RPMIsAppRegistered(appPath, alreadyRegistered)) && alreadyRegistered)
	{
		LOG_INFO(L"RPM app already registered: " << szProcessName);		
	}
	else
	{
		result = m_pDRmcInstance->RPMRegisterApp(appPath, LOGIN_PASSCODE);
		CHK_ERROR_RETURN(!result, false, L"[RPM]Cannot initialize RMX. RPMRegisterApp failed: '%s'", szProcessName);
	}
	
	// Inform PRM this RMX is running now.
	result = m_pDRmcInstance->RPMNotifyRMXStatus(true, LOGIN_PASSCODE);
	CHK_ERROR_RETURN(!result, false, L"[RPM]Cannot initialize RMX. RPMNotifyRMXStatus(running=true) failed (error: %s)", result.ToString().c_str());

	LOG_INFO(L"RMX is running");
	m_sysProjTenantId = m_pDRmUser->GetSystemProjectTenantId();;
	m_sysProjMemberId = m_pDRmUser->GetMembershipID(m_sysProjTenantId);

	DumpSkyDrmInfo();
	m_init = true;
	return true;
}

void CRMXInstance::Terminate()
{
	if (m_pDRmcInstance != nullptr)
	{
		if (m_init)
		{
			CRMXLastResult result;
			result = m_pDRmcInstance->RPMNotifyRMXStatus(false, LOGIN_PASSCODE);
			CHK_ERROR(!result, L"[RPM]RPMNotifyRMXStatus(running=false) failed (error: %s)", result.ToString().c_str());
		}	

		SDWLibDeleteRmcInstance(m_pDRmcInstance);
		m_pDRmcInstance = nullptr;
	}
	
	m_pDRmUser = nullptr;
	m_pDRmTenant = nullptr;
	m_init = false;
}

void CRMXInstance::DumpSkyDrmInfo()
{
	LOG_INFO(L"SkyDRM Information");
	DWORD dwVer = SDWLibGetVersion();
	DWORD dwMajorVer = (DWORD)(HIBYTE(HIWORD(dwVer)));
	DWORD dwMinorVer = (DWORD)(LOBYTE(HIWORD(dwVer)));
	DWORD dwBuild = (DWORD)(LOWORD(dwVer));
	LOG_INFO_FMT(L"\t-Version: %lu.%lu.%lu", dwMajorVer, dwMinorVer, dwBuild);

	LOG_INFO(L"\t-User name: " << m_pDRmUser->GetName().c_str());

	CRMXLastResult result;
	wstring router, tenant, workDir, tempDir, sdkLibDir;
	bool blogin = false;	
	result = m_pDRmcInstance->RPMGetCurrentUserInfo(router, tenant, workDir, tempDir, sdkLibDir, blogin);
	CHK_ERROR(!result, L"[RPM]RPMGetCurrentUserInfo failed (error: %s)", result.ToString().c_str());
	LOG_INFO(L"\t-Working dir: " << workDir.c_str());
	LOG_INFO(L"\t-Temp dir:  " << tempDir.c_str());
	LOG_INFO(L"\t-SdkLib dir:  " << sdkLibDir.c_str());

	LOG_INFO(L"\t-Tenant RMS URL: " << m_pDRmTenant->GetRMSURL().c_str());
	LOG_INFO(L"\t-Tenant router URL: " << m_pDRmTenant->GetRouterURL().c_str());

	LOG_INFO(L"\t-System project tenant id: " << RMXUtils::s2ws(m_sysProjTenantId).c_str());
	LOG_INFO(L"\t-System project membership id: " << RMXUtils::s2ws(m_sysProjMemberId).c_str());

	/*string defaultTenant = m_pDRmUser->GetDefaultTokenGroupName();
	if (!defaultTenant.empty())
	{
		string policies;
		m_pDRmUser->GetPolicyBundle(RMXUtils::s2ws(defaultTenant), policies);
		if (!policies.empty())
		{
			LOG_INFO(L"\t-Policy bundle: ");
			LOG_INFO(RMXUtils::s2ws(policies).c_str());
		}
	}*/
}
