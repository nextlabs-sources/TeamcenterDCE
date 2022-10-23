#pragma once
#include <string>
#ifdef WNT
#include <tchar.h>
#endif
#include <codecvt>

namespace nxl
{
#ifdef UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif
}

// convert UTF-8 string to wstring
inline std::wstring utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
inline std::string wstring_to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}
class NxlString
{
public:
	//constructors
	NxlString() {}
	NxlString(const std::string &str) :_string(str) {
		_wstring = utf8_to_wstring(str);
	}
	NxlString(const std::wstring &wstr) :_wstring(wstr) {
		_string = wstring_to_utf8(wstr);
	}
	NxlString(const char *cstr) {
		if (cstr != nullptr) {
			_string = cstr;
			_wstring = utf8_to_wstring(_string);
		}
	}
	NxlString(const wchar_t *cwstr) {
		if (cwstr != nullptr)
		{
			_wstring = cwstr;
			_string = wstring_to_utf8(_wstring);
		}
	}
	//copy
	NxlString(const NxlString &nstring) :_string(nstring._string), _wstring(nstring._wstring) {
	}
	//assign
	NxlString &operator=(const NxlString &path) {
		_string = path._string;
		_wstring = path._wstring;
		return *this;
	}
	//==
	inline bool operator==(const NxlString &nstring) const {
		return _string == nstring._string;
	}
	inline bool operator==(const std::string &str)const {
		return _string == str;
	}
	inline bool operator==(const std::wstring &wstr)const {
		return _wstring == wstr;
	}
	inline bool operator==(const char *chars)const {
		return chars == nullptr ? _string.length() == 0 : strcmp(_string.c_str(), chars)==0;
	}
	inline bool operator==(const wchar_t*wchars)const {
		return wchars == nullptr ? _wstring.length() == 0 : wcscmp(_wstring.c_str(), wchars)==0;
	}
	//!=
	inline bool operator!=(const NxlString& nstring) const {
		return _string != nstring._string;
	}
	inline bool operator!=(const std::string& str)const {
		return _string != str;
	}
	inline bool operator!=(const std::wstring& wstr)const {
		return _wstring != wstr;
	}
	inline bool operator!=(const char* chars)const {
		return chars == nullptr ? _string.length() > 0 : strcmp(_string.c_str(), chars) != 0;
	}
	inline bool operator!=(const wchar_t* wchars)const {
		return wchars == nullptr ? _wstring.length() > 0 : wcscmp(_wstring.c_str(), wchars) != 0;
	}

	inline const char *chars() const {
		return _string.c_str();
	}
	inline const wchar_t *wchars() const {
		return _wstring.c_str();
	}
	inline const std::string &string() const {
		return _string;
	}
	inline const std::wstring &wstring() const {
		return _wstring;
	}

	inline bool isempty() const {
		return _string.empty();
	}
	inline size_t length() const {
		return _string.length();
	}
	static bool EndsWith(const wchar_t* path, const wchar_t* suffix) {
		if (path == nullptr)
		{
			return suffix == nullptr;
		}
		if (suffix == NULL)
		{
			return true;
		}
		size_t sufLen = wcslen(suffix);
		if (sufLen == 0)
		{
			return true;
		}
		size_t pathLen = wcslen(path);
		if (pathLen < sufLen)
		{
			return false;
		}
		return _wcsicmp((path + (pathLen - sufLen)), suffix) == 0;
	}
	static bool EndsWith(const char* path, const char* suffix) {
		if (path == nullptr)
		{
			return suffix == nullptr;
		}
		if (suffix == NULL)
		{
			return true;
		}
		size_t sufLen = strlen(suffix);
		if (sufLen == 0)
		{
			return true;
		}
		size_t pathLen = strlen(path);
		if (pathLen < sufLen)
		{
			return false;
		}
		return _stricmp((path + (pathLen - sufLen)), suffix) == 0;
	}

#ifdef UNICODE
	inline const TCHAR *tchars() const {
		return _wstring.c_str();
	}
	inline const nxl::tstring &tstring() const {
		return _wstring;
	}
#else
	inline const TCHAR *tchars() const {
		return _string.c_str();
	}
	inline const nxl::tstring &tstring() const {
		return _string;
	}
#endif
protected:
	std::string _string;
	std::wstring _wstring;
};
