#include "RMXInstance.h"

// System
#include <windows.h>

// SkyDRM
#include <SDLAPI.h>

#include <iostream>

using namespace std;

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
	 
	string passcode(LOGIN_PASSCODE);
	SDWLibInit();
	SDWLResult result = RPMGetCurrentLoggedInUser(passcode, m_pDRmcInstance, m_pDRmTenant, m_pDRmUser);
	if (!result || m_pDRmcInstance == nullptr || m_pDRmUser == nullptr || m_pDRmTenant == nullptr)
	{
		printf("Failed to obtain current logged in user from SkyDRM (error: %s)", result.ToString().c_str());
		m_init = false;
		return false;
	}

	m_init = true;
	return true;
}

void CRMXInstance::Terminate()
{
	if (m_pDRmcInstance != nullptr)
	{
		if (m_init)
		{
			SDWLResult result;
			result = m_pDRmcInstance->RPMNotifyRMXStatus(false, LOGIN_PASSCODE);
		}	

		SDWLibDeleteRmcInstance(m_pDRmcInstance);
		m_pDRmcInstance = nullptr;
	}
	
	m_pDRmUser = nullptr;
	m_pDRmTenant = nullptr;
	m_init = false;
}

