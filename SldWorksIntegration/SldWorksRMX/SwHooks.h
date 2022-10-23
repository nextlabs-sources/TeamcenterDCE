#pragma once

class CSldWorksRMX;

enum class PROCESS_EXEC_REASONS {
	PROCESS_NO_RUN = 1,
	PROCESS_POST_FILEPROTECT = 2,
	PROCESS_RUN_NOACTION = 4,
	DEFER_PROCESS_RUN = 8,
	WAIT_FOR_PROCESS_COMPLETION = 0x10
};

class HookInitializer
{
private:
	static CSldWorksRMX *userAddin;

public:
	static void Init(CSldWorksRMX* addinPtr) {
		userAddin = addinPtr;
	}
	static void Startup();
	static void Shutdown();
	static CSldWorksRMX * GetUserAddin() {
		return userAddin;
	}

	static void InstallSwimHook();
	static void UninstallSwimHook();
};

