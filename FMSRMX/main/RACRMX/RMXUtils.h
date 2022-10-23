#define _CRT_SECURE_NO_WARNINGS 

#pragma once

#include <algorithm>
#include <locale>         // std::locale, std::ctype, std::use_facet
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace RMXUtils
{
	/*!
	 * Coverts bite string to wide string
	 * 
	 */
	std::wstring s2ws(const std::string& str);
	/*!
	* Coverts bite string to wide string
	*
	*/
	std::wstring s2ws(const char* szStr);
	/*!
	 * Coverts wide string to bite string
	 * 
	 */
	std::string ws2s(const std::wstring& wstr);
	/*!
	* Coverts wide string to bite string
	*
	*/
	std::string ws2s(const wchar_t* szStr);
	/*!
	 * Returns a string representation of current time (local dependent)
	 * e.g. Sun Oct 17 04:41:13 2010
	 */
	std::wstring getCurrentTime();
	/*!
	 * Returns the value of specified env var
	 * 
	 */
	std::wstring getEnv(const std::wstring& envName);
	/*!
	 * Creates a new process to run the specified command line, and returns the result.
	 * 
	 */
	bool ExecuteCmd(const std::wstring& cmd, std::string& output);

	/*!
	* Generates a unique UUID
	*/
	std::wstring GenerateUUID();

	/*!
	* Returns command lines for the current process
	*/
	std::vector<std::wstring> GetCommandLines();

	/*!
	* Returns the file full path for the given file handler
	*/
	std::wstring GetFilePathByHandle(HANDLE hFile);

	/*!
	* Retrieves the current directory for the current process
	*/
	std::wstring GetCurrentDir();

	/*!
	* Returns the full path of image for the current process
	*/
	std::wstring GetCurrentProcessImagePath();

	/*!
	* Find the main window handlers for the given process
	*/
	std::vector<HWND> FindProcessMainWindow(DWORD dwProcessId);

	/*!
	* Find the main window handlers for current process
	*/
	std::vector<HWND> FindCurrentProcessMainWindow();

	/*!
	* Converts the string c to lower case if a lower case form is defined by this locale
	*/
	template<typename CharT>
	void ToLower(std::basic_string<CharT>& s, const std::locale& loc = std::locale()) {
		auto& f = std::use_facet<std::ctype<CharT>>(loc);
		std::transform(s.begin(), s.end(), s.begin(),
			[&](CharT c) -> CharT { return f.tolower(c); });
	}

	/*!
	* Converts the string c to upper case if a lower case form is defined by this locale
	*/
	template<typename CharT>
	void ToUpper(std::basic_string<CharT>& s, const std::locale& loc = std::locale()) {
		auto& f = std::use_facet<std::ctype<CharT>>(loc);
		std::transform(s.begin(), s.end(), s.begin(),
			[&](CharT c) -> CharT { return f.toupper(c); });
	}

	/*!
	* Replace all occurrences of the string by new contents with case insensitive
	*/
	template<class CharT>
	void IReplaceAllString(std::basic_string<CharT>& s, const std::basic_string<CharT>& oldValue, const std::basic_string<CharT>& newValue)
	{
		size_t pos = 0;
		std::basic_string<CharT> lStr(s);
		std::basic_string<CharT> lOldValue(oldValue);
		RMXUtils::ToLower<CharT>(lStr);
		RMXUtils::ToLower<CharT>(lOldValue);
		while ((pos = lStr.find(lOldValue, pos)) != std::basic_string<CharT>::npos) {
			lStr.replace(pos, oldValue.length(), newValue);
			s.replace(pos, oldValue.length(), newValue);
			pos += newValue.length();
		}
	}

	/*!
	* Find the first string in case insensitive
	*/
	template<class CharT>
	size_t IFindSubString(std::basic_string<CharT>& str, const std::basic_string<CharT>& subStr, size_t pos = 0)
	{
		std::basic_string<CharT> lStr(str);
		std::basic_string<CharT> lSubStr(sbuStr);
		RMXUtils::ToLower<CharT>(lStr);
		RMXUtils::ToLower<CharT>(lSubStr);
		return lStr.find(lSubStr, pos);
	}

	/*!
	* Retrieves DNS name associated with the local computer.
	* If the local computer is in a DNS domain, returns the fully qualified DNS name, using the form HostName.DomainName.
	* Otherwise, returns host name only.
	*/
	std::wstring GetHostName();

	/*!
	* Retrieves ip address with the local computer.
	*/
	std::wstring GetHostIP();

	/*!
	* Format args according to the format string, and return the result as a string
	*/
	std::string FormatString(const char * format, ...);

	/*!
	* Format args according to the format string, and return the result as a string
	*/
	std::wstring FormatString(const wchar_t * format, ...);

	/*!
	 * Remove all leading and trailing spaces from the input. 
	 * The input sequence is modified in-place
	 */
	std::string& Trim(std::string& s);

	/*!
	* Remove all leading and trailing spaces from the input.
	* The input sequence is modified in-place
	*/
	std::wstring& Trim(std::wstring& ws);

	template <typename F>
	class ScopeGuard {
	public:
		explicit ScopeGuard(F&& f) _NOEXCEPT : m_f(std::forward<F>(f)) {}
		~ScopeGuard() _NOEXCEPT { m_f(); }
		
		ScopeGuard(const ScopeGuard&& other) _NOEXCEPT : m_f(std::move(other.m_f)) {}
		ScopeGuard & operator=(ScopeGuard && other) _NOEXCEPT {
			m_f = std::move(other.m_f);
		}
	private:
		F m_f;
		ScopeGuard() {};
		ScopeGuard(const ScopeGuard &) {};
		ScopeGuard &operator=(const ScopeGuard &) {};
	};

	template <typename T>
	ScopeGuard<T> MakeScopeGuard(T&& f)
	{
		return ScopeGuard<T>(std::forward<T>(f));
	}

}; // namespace RMXUtils

#define RMX_STR_CAT(a, b) a##b
#define RMX_SCOPE_GUARD_NAME(name, line) RMX_STR_CAT(name, line)
#define RMX_SCOPE_GUARD(f) const auto& RMX_SCOPE_GUARD_NAME(scopeGuard, __LINE__) = RMXUtils::MakeScopeGuard(f)
#define RMX_SCOPE_GUARD_ENTER(pInProgress) \
*pInProgress = true; \
RMX_SCOPE_GUARD([&]() { *pInProgress = false; });
