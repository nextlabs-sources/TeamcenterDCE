#pragma once
class AcRMXHooks
{
public:
	static void InstallGlobalHooks();
	static void UnhookGlobalHooks();

	static void Hook_NavDlgOK(bool creatingDoc = false);
	static void Unhook_NavDlgOK();

	static void Hook_CreateFile();
	static void Unhook_CreateFile();
};

