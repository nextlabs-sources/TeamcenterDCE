// rmxunitlity.cpp : Defines the entry point for the console application.
#include "CommonUtils.h"

#include "SDRmSession.h"

using namespace std;

void  printUsage(bool showDetails)
{
	wcout << L"Usage: rmxutility.exe" << endl;
	wcout << L"\t[-registerapp <appPath>]" << endl;
	wcout << L"\t[-unregisterapp <appPath>]" << endl;
	wcout << L"\t[-isappregistered <appPath>]" << endl;
	wcout << L"\t[-addrpmdir <dir>]" << endl;
	wcout << L"\t[-rmrpmdir <dir>]" << endl;
	wcout << L"\t[-isrpmdir <dir>]" << endl;
	wcout << L"\t[-lsrpmdir <dir>]" << endl;
	wcout << L"\t[-isloggedin]" << endl;
	//wcout << L"\t[-lspolicy]" << endl;
	wcout << L"\t[-sdkver]" << endl;
	wcout << L"\nFor more help, type 'rmxutility.exe -help'\n" << endl;

	if (showDetails)
	{
		wcout << L"Options:" << endl;
		wcout << L"-registerapp <dir>\n\tRegisters an application that is controlled by RMX" << endl;
		wcout << L"-unregisterapp <dir>\n\tUnregisters an application that is controlled by RMX" << endl;
		wcout << L"-isappregistered <dir>\n\tChecks if the application is registered as RPM trusted or not" << endl;
		wcout << L"-addrpmdir <dir>\n\tAdds a directory as RPM folder" << endl;
		wcout << L"-rmrpmdir <dir>\n\tRemoves a directory as RPM folder" << endl;
		wcout << L"-isrpmdir <dir>\n\tChecks if the specified directory is an RPM folder" << endl;
		wcout << L"-lsrpmdir <dir>\n\tLists all existing RPM sub folders for the specified directory" << endl;
		wcout << L"-isloggedin <dir>\n\tChecks if the user has logged in SkyDRM Desktop" << endl;
		//wcout << L"-lspolicy \n\tLists all policies" << endl;
		wcout << L"-sdkVer \n\tPrints SkyDRM Client SDK Version" << endl;
	}	
}

const static wchar_t* ACTION_ISLOGGEDIN = L"isloggedin";
const static wchar_t* ACTION_REGISTERAPP = L"registerapp";
const static wchar_t* ACTION_UNREGISTERAPP = L"unregisterapp";
const static wchar_t* ACTION_ISAPPREGISTERD = L"isappregistered";
const static wchar_t* ACTION_ADDRPMDIR = L"addrpmdir";
const static wchar_t* ACTION_REMOVERPMDIR = L"rmrpmdir";
const static wchar_t* ACTION_ADDTRUSTEDPROCESS = L"addtrust";
const static wchar_t* ACTION_ISRPMDIR = L"isrpmdir";
const static wchar_t* ACTION_LSRPMDIR = L"lsrpmdir";
const static wchar_t* ACTION_LSPOLICY = L"lspolicy";
const static wchar_t* ACTION_SDKVER = L"sdkVer";

int _tmain(int argc, _TCHAR* argv[])
{
	bool validArgs = false;
	if ((argc > 1) && ((*argv[1] == L'-' || (*argv[1] == L'/'))))
	{
		if (argc > 2)
		{
			const wchar_t* action = argv[1] + 1;
			if (_wcsicmp(ACTION_REGISTERAPP, action) == 0)
			{
				validArgs = true;
				CSDRmSession sess;
				if (!sess.RPMRegisterApp(argv[2]))
					exit(1);
			}
			else if (_wcsicmp(ACTION_UNREGISTERAPP, action) == 0)
			{
				validArgs = true;
				CSDRmSession sess;
				if (!sess.RPMUnregisterApp(argv[2]))
					exit(1);
			}
			else if (_wcsicmp(ACTION_ISAPPREGISTERD, action) == 0)
			{
				validArgs = true;
				CSDRmSession sess;
				bool bRegistered = false;
				if (!sess.RPMIsAppRegistered(argv[2], bRegistered))
				{
					exit(1);
				}
				const wchar_t* retMsg = bRegistered ? L"yes" : L"no";
				LOG_INFO(retMsg);
			}
			else if (_wcsicmp(ACTION_ADDRPMDIR, action) == 0)
			{
				validArgs = true;
				CSDRmSession sess;
				if (!sess.AddRPMDir(argv[2]))
					exit(1);
			}
			else if (_wcsicmp(ACTION_REMOVERPMDIR, action) == 0)
			{
				validArgs = true;
				CSDRmSession sess;
				if (!sess.RemoveRPMDir(argv[2]))
					exit(1);
			}
			else if (_wcsicmp(ACTION_ISRPMDIR, action) == 0)
			{
				validArgs = true;
				CSDRmSession sess;
				bool isRPMDir = false;
				if (!sess.IsRPMDir(argv[2], isRPMDir))
				{
					exit(1);
				}
			}
			else if (_wcsicmp(ACTION_LSRPMDIR, action) == 0)
			{
				validArgs = true;
				CSDRmSession sess;
				vector<wstring> dirs;
				if (!sess.GetAllRPMDirs(argv[2], dirs))
					exit(1);
			}
		}
		else {
			if (_wcsicmp(ACTION_ISLOGGEDIN, argv[1] + 1) == 0)
			{
				validArgs = true;
				CSDRmSession sess;
				if (!sess.IsLoggedIn())
				{
					LOG_INFO(L"no");
					exit(1);
				}
				else
				{
					LOG_INFO(L"yes");
				}				
			}
			/*else if (_wcsicmp(ACTION_LSPOLICY, argv[1] + 1) == 0)
			{
				validArgs = true;
				CSDRmSession sess;
				string policies;
				if (!sess.GetPolicyBundle(policies))
					exit(1);
			}*/
			else if (_wcsicmp(ACTION_SDKVER, argv[1] + 1) == 0)
			{
				validArgs = true;
				CSDRmSession::GetSDKVersion();
			}
			else if (_wcsicmp(L"help", argv[1] + 1) == 0)
			{
				validArgs = true;
				printUsage(true);
			}
		}
	}

	if (!validArgs)
		printUsage(false);

    return 0;
}

