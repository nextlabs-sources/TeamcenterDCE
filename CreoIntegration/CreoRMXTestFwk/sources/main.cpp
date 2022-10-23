#include "XTestLogger.h"

#include <PathEx.h>
#include "TestFwk.h"

extern "C" int user_initialize(
	int argc,
	char *argv[],
	char *version,
	char *build,
	wchar_t errbuf[80])
{  
	//if (!GetModuleHandle(L"CreoRMX_x64.dll") && !GetModuleHandle(L"CreoRMX4_x64.dll"))
	//{
	//	return (wfcTK_APP_INIT_FAIL);
	//}
	const wstring& fkRootDir = TestFwk().GetInstallDir();
	if (fkRootDir.empty())
		return (wfcTK_GENERAL_ERROR);

	CPathEx::RemoveFile(TestFwk().GetTestLogError()); 
	CPathEx::RemoveFile(TestFwk().GetTestLogInfo());

	LOG_BEGIN(fkRootDir);
	
	if (!TestFwk().Init())
	{
		XLOG_ERROR("Failed to init test framework. Unload Plugin.");
		return (wfcTK_GENERAL_ERROR);
	}

	return (wfcTK_NO_ERROR);
}

extern "C" void user_terminate()
{
	LOG_END();
}
