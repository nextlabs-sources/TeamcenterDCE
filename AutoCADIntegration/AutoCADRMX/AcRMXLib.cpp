#include "StdAfx.h"
#include "AcRMXLib.h"
#include "AcRMXAccessControl.h"
#include "AcRXMProtectControl.h"

#define AC_LOG_ENTER(msg) AC_LOG_ENTER_BODY(L"ACRMXAPI", msg) // log guard helper

ACRMXAPI bool AcRMX_ProtectExportFile(const wchar_t* szFileToProtect)
{
	AC_LOG_ENTER(szFileToProtect);

	if (ProtectController().AddExportJob(szFileToProtect))
		return ProtectController().ExecuteJob(szFileToProtect);

	return false;
}

ACRMXAPI bool AcRMX_CheckRights(unsigned long dwRights, bool bIncludRefs, bool bAlert)
{
	AC_LOG_ENTER(L"");
	auto rStatus = CAcRMXAccessControl::CheckRight(acDocManager->mdiActiveDocument(), dwRights, bIncludRefs, bAlert);
	return (rStatus == AcRMX::eRightGrant);
}
