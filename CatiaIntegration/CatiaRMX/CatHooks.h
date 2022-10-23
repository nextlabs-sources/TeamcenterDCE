#pragma once
class CCatHooks
{
public:
	static void InstallHooks();
	static void UninstallHooks();

	static void Start();
	static void Stop();
};

