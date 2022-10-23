#pragma once

namespace SwimUtils
{
	bool FindPropertyFile(const wstring& filePath, wstring &swimPropertyFilePath);
	void ReadSwimProperty(const wstring&filePath, const wstring& property, wstring&value);
	bool GetSwimPropertyValue(const wstring& property, wstring& value);
	DWORD GetSwimPID();
	bool SetSwimPIDAsTrusted();
	void InitSwim();
	bool RefreshSwimRegistry(const wstring& regFile);
	wstring GetSW2CacheFolder();
};