// NXDRMIntegration.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NXDRMIntegration.h"
#include <Shlwapi.h>
#include <ShlObj.h>
#include <NxlUtils/SysUtils.hpp>

#define RPM_MAGIC_SECURITY "{6829b159-b9bb-42fc-af19-4a6af3c9fcf6}"

static std::shared_ptr<const DISW::DRM::Host::ISessionInformation> sessionInfoInterface;

static RPMSession *rpmSession = nullptr;

//----------------------------------------------------------------------------------------------------------------------------------
static DISW::DRM::Adaptor::IDRM* getIDRMInterface()
{
	static NXL::NX::DRM::AdaptorImpl adaptor;
	return &adaptor;
}
//----------------------------------------------------------------------------------------------------------------------------------
DISW::DRM::Adaptor::IDRM* NXL::NX::DRM::Initialize
(
	std::shared_ptr<const DISW::DRM::Host::ISessionInformation> interf
)
{
	sessionInfoInterface = interf;
	rpmSession = new RPMSession(RPM_MAGIC_SECURITY);
	rpmSession->ListRPMInfo();
	rpmSession->ListPolicies();
	//set trust APP
	//rpmSession->RegisterApp(true);
	rpmSession->SetRMXStatus(true);
	//set FCCCache as rpm folder
	bool ismanaged = GetISessionInformation()->IsManagedMode();
	DBGLOG("IsManaged=%d", ismanaged);
	if (ismanaged) {
		WCHAR szPath[MAX_PATH];
		//set FCC root folder as RPMDir
		if (SUCCEEDED(SHGetFolderPathW(NULL,
			CSIDL_PROFILE,	//c:\users\<username>
			NULL,
			0,
			szPath)))
		{
			PathAppend(szPath, L"FCCCache");
			bool added;
			if (rpmSession->AddRPMFolder(szPath, &added) == 0)
			{
				if (added) {
					//set FCCCache as RPMDir
					DBGLOG("Added '%s' as RPM safe directory", szPath);
				}
				else
				{
					DBGLOG("'%s' is already RPM safe diretory", szPath);
				}
			}
		}
	}
	//set UGII_TMP_DIR as rpm folder
#define ENV_SET_RPM TEXT("SET_UGII_TMP_DIR_AS_RPM")
	auto setTmpAsRPM = SysUtils::GetEnvVariable(ENV_SET_RPM);
	if (setTmpAsRPM.length() > 0)
	{
		DBGLOG("%s=%s", ENV_SET_RPM, setTmpAsRPM.c_str());
		if (_tcsicmp(setTmpAsRPM.c_str(), TEXT("1")) == 0)
		{
			//set UGII_TMP_DIR as rpm folder
			auto nxtmp = SysUtils::GetEnvVariable(TEXT("UGII_TMP_DIR"));
			if (!nxtmp.empty())
			{
				bool added;
				if (rpmSession->AddRPMFolder(nxtmp.c_str(), &added) == 0)
				{
					if (added) {
						//set FCCCache as RPMDir
						INFOLOG("Added %%UGII_TMP_DIR%%('%s') as RPM safe directory", nxtmp.c_str());
					}
					else
					{
						DBGLOG("%%UGII_TMP_DIR%%('%s') is already RPM safe diretory", nxtmp.c_str());
					}
				}
			}
		}
	}
	else
	{
		WARLOG("Environment variable-%%" ENV_SET_RPM "%% is not defined, please make sure %%UGII_TMP_DIR%% has been set as RPM folder manually");
	}
	return getIDRMInterface();
}

//----------------------------------------------------------------------------------------------------------------------------------
extern std::shared_ptr<const DISW::DRM::Host::ISessionInformation> NXL::NX::DRM::GetISessionInformation()
{
	return sessionInfoInterface;
}
extern RPMSession *NXL::NX::DRM::GetRPMSession() {
	return rpmSession;
}
