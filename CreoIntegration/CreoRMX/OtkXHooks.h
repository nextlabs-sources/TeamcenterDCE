#pragma once

class HookInitializer
{
public:
	static void Startup();
	static void Shutdown();

	static void InstallHook_Export();
	static void UnstallHook_Export();
};

