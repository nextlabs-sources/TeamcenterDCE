#pragma once
#include <iostream>
#include <SDWL/SDLAPI.h>

bool report_rpm_error(const SDWLResult &ret, const char *func, const char *file, int line, const char *call)
{
	if (ret == (int)SDWL_SUCCESS)
	{
		return true;
	}
	//TODO:logging
	std::cout << "RPMAPI Failed(Code=" << ret.GetCode()
		<< L" Message='" << ret.GetMsg() << "' in "
		<< ret.GetFunction() << "()@file-" << ret.GetFile() << ")" << std::endl;
	return false;
}

#define RPM_CALL(x) report_rpm_error(x, __FUNCTION__, __FILE__, __LINE__, #x)

#define RPM_PASSCODE "{6829b159-b9bb-42fc-af19-4a6af3c9fcf6}"

class RPMTester {
public:
	RPMTester() :_passcode(RPM_PASSCODE), _pTenant(nullptr), _pUser(nullptr), _pInstance(nullptr)
	{
		RPM_CALL(RPMGetCurrentLoggedInUser(_passcode, _pInstance, _pTenant, _pUser));
	}
	inline bool initialized() const { return _pUser != nullptr; }
	inline ISDRmTenant * Tenant() { return _pTenant; }
	inline ISDRmUser *User() { return _pUser; }
	inline ISDRmcInstance *Instance() { return _pInstance; }

private:
	std::string _passcode;
	ISDRmTenant *_pTenant;
	ISDRmUser *_pUser;
	ISDRmcInstance *_pInstance;
};
