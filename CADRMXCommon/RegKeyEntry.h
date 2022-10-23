#pragma once

#include <string>
#include <vector>
#include <Windows.h>

#define REG_SUCCEEDED(ret) (ret == ERROR_SUCCESS)
#define REG_FAILED(ret) (ret != ERROR_SUCCESS)

class CRegKeyEntry
{
public:
	CRegKeyEntry();
	explicit CRegKeyEntry(HKEY hKey);
	virtual ~CRegKeyEntry();

public:
	/*!
	 * Opens the specified registry key
	 * 
	 * \param hKey: A handle to an open registry key.
	 * \param strSubKey: The name of the registry subkey to be opened.
	 * \param samDesired: A mask that specifies the desired access rights to the key to be opened. 
	 * \return If the function succeeds, the return value is ERROR_SUCCESS.
	 */
	LONG Open(HKEY hKey, const std::wstring& strSubKey, REGSAM samDesired = KEY_READ);
	/*!
	 * Closes a handle to the opened registry key.
	 * 
	 * \return 
	 */
	LONG Close();
	/*!
	 * Creates the specified registry key. If the key already exists in the registry, the function opens it.
	 * 
	 * \param hKey
	 * \param strSubKey
	 * \param dwOptiosn
	 * \param samDesired
	 * \param lpSecAttr
	 * \param lpdwDisposition
	 * \return If the function succeeds, the return value is ERROR_SUCCESS.
	 */
	LONG Create(HKEY hKey,
		const std::wstring& strSubKey,
		DWORD dwOptiosn = REG_OPTION_NON_VOLATILE,
		REGSAM samDesired = KEY_WRITE,
		LPSECURITY_ATTRIBUTES lpSecAttr = NULL,
		LPDWORD lpdwDisposition = NULL);
	/*!
	 * Retrieves the multi-string values for the specified value name associated with an open registry key.
	 * 
	 * \param strValueName
	 * \param values
	 * \return 
	 */
	LONG QueryMultiStringValue(const std::wstring& strValueName, std::vector<std::wstring>& values);
	/*!
	 *  Set the multi-string values for the specified value name associated with an open registry key.
	 * 
	 * \param strValueName
	 * \param values
	 * \return 
	 */
	LONG SetMultiStringValue(const std::wstring& strValueName, const std::vector<std::wstring>& values);
	/*!
	* Retrieves the multi-string values for the specified value name associated with an open registry key.
	* This is one-step api to open the key, set the value, and close the key
	* \param strValueName
	* \param values
	* \return
	*/
	LONG QueryMultiStringValue(HKEY hKey,
		const std::wstring& strSubKey,
		const std::wstring& strValueName,
		REGSAM samDesired,
		std::vector<std::wstring>& values);
	/*!
	*  Set the multi-string values for the specified value name associated with an open registry key.
	*  This is one-step api to open the key, set the value, and close the key
	* \param strValueName
	* \param values
	* \return
	*/
	LONG SetMultiStringValue(HKEY hKey,
		const std::wstring& strSubKey,
		const std::wstring& strValueName,
		const std::vector<std::wstring>& values,
		REGSAM samDesired = KEY_WRITE
		);

	/*!
	* Retrieves the string value for the specified value name associated with an open registry key.
	* This is one-step api to open the key, set the value, and close the key
	* \param strValueName
	* \param values
	* \return
	*/
	LONG QueryStringValue(HKEY hKey,
		const std::wstring& strSubKey,
		const std::wstring& strValueName,
		REGSAM samDesired,
		std::wstring& value);
private:
	HKEY m_hKey;
};


