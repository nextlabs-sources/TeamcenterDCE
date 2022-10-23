#include "RegistryHelper.h"

namespace RegistryHelper {
	bool GetLongValFromReg(HKEY keyParent, CString keyName, CString keyValName, DWORD &dwVal)
	{
		CRegKey key;
		bool isKeyOpen;
		isKeyOpen = OpenRegKey(keyParent, keyName, KEY_READ, key);
		if (!isKeyOpen) {
			return false;
		}

		DWORD status;
		status = key.QueryDWORDValue(keyValName, dwVal);
		if (status != ERROR_SUCCESS) {
			LOG_ERROR_FMT("Error reading keyName = %s, KeyValName = %s, ErrorCode =%u", keyName, keyValName, status);
			return false;
		}
		key.Close();
		return true;

	}

	bool GetStringValFromReg(HKEY keyParent, CString keyName, CString keyValName, wstring &strVal) {
		CRegKey key;
		bool isKeyOpen;
		isKeyOpen = OpenRegKey(keyParent, keyName, KEY_READ, key);
		if (!isKeyOpen) {
			return false;
		}

		ULONG nValueLength = 0;
		DWORD status;
		status = key.QueryStringValue(keyValName, NULL, &nValueLength);
		if (status != ERROR_SUCCESS) {
			LOG_ERROR_FMT("Error reading keyName = %s, KeyValName = %s, ErrorCode =%u", keyName, keyValName, status);
			return false;
		}
		CString sValue;
		if (nValueLength > 0) {
			status = key.QueryStringValue(keyValName, sValue.GetBufferSetLength(nValueLength - 1), &nValueLength);
			if (status != ERROR_SUCCESS) {
				LOG_ERROR_FMT("Error reading keyName = %s, KeyValName = %s, ErrorCode =%u", keyName, keyValName, status);
				return false;
			}
		}
		strVal = wstring(sValue.GetString());
		key.Close();
		return true;

	}


	bool SetLongValToReg(HKEY keyParent, CString keyName, CString keyValName, DWORD dwValue) {
		CRegKey key;
		bool isKeyOpen;
		isKeyOpen = OpenRegKey(keyParent, keyName, KEY_READ | KEY_WRITE, key);
		if (!isKeyOpen) {
			return false;
		}
		DWORD status;
		status = key.SetDWORDValue(keyValName, dwValue);
		if (status != ERROR_SUCCESS) {
			LOG_ERROR_FMT("Error reading keyName = %s, KeyValName = %s, ErrorCode =%u", keyName, keyValName, status);
			return false;
		}
		key.Close();
		return true;
	}


	bool SetStringValToReg(HKEY keyParent, CString keyName, CString keyValName, CString value) {
		CRegKey key;
		bool isKeyOpen;
		isKeyOpen = OpenRegKey(keyParent, keyName, KEY_READ | KEY_WRITE, key);
		if (!isKeyOpen) {
			return false;
		}
		DWORD status;
		status = key.SetStringValue(keyValName, value);
		if (status != ERROR_SUCCESS) {
			LOG_ERROR_FMT("Error reading keyName = %s, KeyValName = %s, ErrorCode =%u", keyName, keyValName, status);
			return false;
		}
		key.Close();
		return true;
	}


	bool OpenRegKey(HKEY keyParent, CString keyName, REGSAM samDesired, CRegKey &hKey) {
		CRegKey key;
		DWORD status;
		status = key.Open(keyParent, keyName, samDesired);
		if (status != ERROR_SUCCESS) {
			LOG_ERROR_FMT("Error reading keyName = %s, ErrorCode =%u", keyName, status);
			return false;
		}
		hKey = key;
		return true;
	}


	bool CreateKey(HKEY hKeyParent, LPCTSTR lpszKeyName, LPTSTR lpszClass, DWORD dwOptions,
		REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecAttr, LPDWORD lpdwDisposition) {
		CRegKey key;
		DWORD status = key.Create(hKeyParent, lpszKeyName, NULL, 0, KEY_ALL_ACCESS, NULL, NULL);
		if (status != ERROR_SUCCESS) {
			LOG_ERROR_FMT("Error creating key . HKEY = %s, KeyName = %s , Error = %d", hKeyParent, lpszKeyName, status);
			key.Close();
			return false;
		}
		key.Close();
		return true;
	}
}