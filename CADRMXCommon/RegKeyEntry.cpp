#include "RegKeyEntry.h"

CRegKeyEntry::CRegKeyEntry()
	: m_hKey(nullptr)
{
}

CRegKeyEntry::CRegKeyEntry(HKEY hKey)
{
	Close();
	m_hKey = hKey;
}


CRegKeyEntry::~CRegKeyEntry()
{
	Close();
}

LONG CRegKeyEntry::Open(HKEY hKey, const std::wstring & strSubKey, REGSAM samDesired)
{
	HKEY hSubKey;
	LONG retResult = RegOpenKeyEx(hKey, strSubKey.c_str(), 0, samDesired, &hSubKey);
	if (REG_SUCCEEDED(retResult))
	{
		m_hKey = hSubKey;
	}

	return retResult;
}

LONG CRegKeyEntry::Close()
{
	LSTATUS retResult = ERROR_SUCCESS;
	if (m_hKey)
	{
		retResult = RegCloseKey(m_hKey);
		m_hKey = nullptr;
	}
	return retResult;
}

LONG CRegKeyEntry::Create(HKEY hKey, const std::wstring & strSubKey, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecAttr, LPDWORD lpdwDisposition)
{
	HKEY hSubKey;
	LONG retResult = RegCreateKeyEx(hKey, strSubKey.c_str(), 0, nullptr, dwOptions, samDesired, lpSecAttr, &hSubKey, lpdwDisposition);
	if (REG_SUCCEEDED(retResult))
	{
		m_hKey = hSubKey;
	}
	
	return retResult;
}

LONG CRegKeyEntry::QueryMultiStringValue(const std::wstring & strValueName, std::vector<std::wstring>& values)
{
	std::vector<wchar_t> data;
	DWORD cbData = 0;
	LONG retResult = RegQueryValueEx(m_hKey, strValueName.c_str(), 0, nullptr, nullptr, &cbData);
	if (REG_SUCCEEDED(retResult))
	{
		data.resize(cbData / sizeof(wchar_t));
		DWORD dwType = REG_MULTI_SZ;
		retResult = RegQueryValueEx(m_hKey, strValueName.c_str(), 0, &dwType, (LPBYTE)&data[0], &cbData);
		if (REG_SUCCEEDED(retResult))
		{
			data.resize(cbData / sizeof(wchar_t));

			const wchar_t* szCurr = &data[0];
			while (*szCurr != L'\0')
			{
				const size_t currLen = wcslen(szCurr);
				values.push_back(std::wstring(szCurr, currLen));
				szCurr += currLen + 1;
			}
		}
	}
	return retResult;
}

LONG CRegKeyEntry::SetMultiStringValue(const std::wstring & strValueName, const std::vector<std::wstring>& values)
{
	std::vector<wchar_t> data;
	if (values.empty())
		data = std::vector<wchar_t>(2, L'\0');
	else
	{
		size_t len = 0;
		for (const auto& v : values)
		{
			len += v.length() + 1;
		}
		len++; // string should be terminated by '\0'

		data.reserve(len);
		for (const auto& v : values)
		{
			data.insert(data.end(), v.begin(), v.end());
			// Add terminator '\0'
			data.push_back(L'\0');
		}
		// Last terminator
		data.push_back(L'\0');		
	}

	const DWORD cbData = (DWORD)(data.size() * sizeof(wchar_t));

	return RegSetValueEx(m_hKey, strValueName.c_str(), 0, REG_MULTI_SZ, (LPBYTE)&data[0], cbData);
}

LONG CRegKeyEntry::QueryMultiStringValue(HKEY hKey, const std::wstring & strSubKey, const std::wstring & strValueName, REGSAM samDesired, std::vector<std::wstring>& values)
{
	LONG retResult = Open(hKey, strSubKey, samDesired);
	if (REG_FAILED(retResult))
		return retResult;

	retResult = QueryMultiStringValue(strValueName, values);
	Close();
	
	return retResult;
}

LONG CRegKeyEntry::SetMultiStringValue(HKEY hKey, const std::wstring & strKey, const std::wstring & strValueName, const std::vector<std::wstring>& values, REGSAM samDesired /*= KEY_WRITE*/)
{
	LONG retResult = Create(hKey, strKey, 0, samDesired);
	if (REG_FAILED(retResult))
		return retResult;

	retResult = SetMultiStringValue(strValueName, values);
	Close();

	return retResult;
}

LONG CRegKeyEntry::QueryStringValue(HKEY hKey, const std::wstring& strSubKey, const std::wstring& strValueName, REGSAM samDesired, std::wstring& value)
{
	LONG retResult = Open(hKey, strSubKey, samDesired);
	if (REG_FAILED(retResult))
		return retResult;

	std::vector<wchar_t> data;
	DWORD cbData = 0;
	retResult = RegQueryValueEx(m_hKey, strValueName.c_str(), 0, nullptr, nullptr, &cbData);
	if (REG_SUCCEEDED(retResult))
	{
		data.resize(cbData / sizeof(wchar_t));
		DWORD dwType = REG_SZ;
		retResult = RegQueryValueEx(m_hKey, strValueName.c_str(), 0, &dwType, (LPBYTE)&data[0], &cbData);
		if (REG_SUCCEEDED(retResult))
		{
			data.resize(cbData / sizeof(wchar_t));

			const wchar_t* szCurr = &data[0];
			while (*szCurr != L'\0')
			{
				const size_t currLen = wcslen(szCurr);
				value = std::wstring(szCurr, currLen);
				break;
			}
		}
	}
	
	Close();

	return retResult;
}
