#pragma once

#include <string>
#include "CommonTypes.h"


// Forward declarations
class ISDRmcInstance;
class ISDRmUser;
class ISDRmTenant;

static const char* LOGIN_PASSCODE = "{6829b159-b9bb-42fc-af19-4a6af3c9fcf6}";

class CRMXInstance
{
	RMX_SINGLETON_DECLARE(CRMXInstance)

public:	
	bool Initialize();
	void Terminate();
	bool IsInit() const {return m_init; }
	ISDRmcInstance* GetDRmcInstance() const { return m_pDRmcInstance; }
	ISDRmUser* GetDRmUser() const { return m_pDRmUser; }
	ISDRmTenant* GetDRmTenant() const { return m_pDRmTenant; }
	std::string GetSystemProjectMemberId() const { return m_sysProjMemberId; }

private:
	ISDRmcInstance* m_pDRmcInstance;
	ISDRmUser* m_pDRmUser;
	ISDRmTenant* m_pDRmTenant;
	std::string m_sysProjTenantId;
	std::string m_sysProjMemberId;
	bool m_init;
};
