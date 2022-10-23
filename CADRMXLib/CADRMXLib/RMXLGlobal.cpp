#include "External\RMXLGlobal.h"

#include <SDLAPI.h>
#include <SDLResult.h>

#include "InternalTypeDef.h"
#include "RMXLLogger.h"
#include <RMXUtils.h>

#include "RMXLInstanceImpl.h"
#include "RMXLResultEx.h"
#include "RMXLUserImpl.h"

static const DWORD RMXL_VERSION = (VER_MAJOR << 24) | (VER_MINOR << 16) | (VER_BUILD << 0);

using namespace RMXLib;

CADRMX_API DWORD RMX_GetLibVersion(void)
{
	RMXL_LOG_ENTER;

	return RMXL_VERSION;
}

CADRMX_API DWORD RMX_GetSDWLibVersion(void)
{
	RMXL_LOG_ENTER;

	return SDWLibGetVersion();
}

CADRMX_API RMXResult RMX_GetCurrentLoggedInUser(IRMXInstance*& pInstance, IRMXUser*& pUser)
{
	RMXL_LOG_ENTER;
	
	CRMXLResultEx result;
	ISDRmcInstance* pDRmcInst = nullptr;
	ISDRmUser* pDRmUser = nullptr;
	ISDRmTenant* pDRmTenant = nullptr;
	static std::string passcode(RMXL_LOGIN_PASSCODE);

	result = RPMGetCurrentLoggedInUser(passcode, pDRmcInst, pDRmTenant, pDRmUser);
	RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMGetCurrentLoggedInUser %s", L"failed");
	if (pDRmcInst == nullptr || pDRmUser == nullptr)
	{ 
		result = RMX_RESULT2(ERROR_INTERNAL_ERROR, L"Invalid pDRmcInst/pDRmUser");
		RMXL_ERROR_RETURN_FMT(!result, result, L"[SDL]RPMGetCurrentLoggedInUser %s", L"failed");
	}
	
	auto instImpl = new CRMXLInstanceImpl(pDRmcInst, pDRmUser);
	pInstance = RMXL_PTRAS(instImpl, IRMXInstance);
	instImpl->GetLoginUser(pUser);

	return result;
}

CADRMX_API void RMX_DeleteRMXInstance(IRMXInstance* pInstance)
{
	RMXL_LOG_ENTER;

	if (pInstance != nullptr)
	{
		auto pInstImpl = RMXL_PTRAS(pInstance, CRMXLInstanceImpl);
		delete pInstImpl;
		pInstImpl = nullptr;
	}
}

CADRMX_API void RMX_FreeArrayMem(void* pArr)
{
	RMXL_LOG_ENTER;

	if (pArr)
	{
		delete[] pArr;
		pArr = nullptr;
	}
}

CADRMX_API void RMX_FreeMem(void* p)
{
	RMXL_LOG_ENTER;

	if (p)
	{
		delete p;
		p = nullptr;
	}
}
