#pragma once
#include <string>
#include <Windows.h>
#include <Shlwapi.h>
#include "NxlString.hpp"
#include "utils_log4cpp.hpp"
#include "SysUtils.hpp"


#define ASSERT_FILE_EXISTS(file) (file).AssertFileExists(TEXT(__FUNCTION__), __LINE__)
class NxlPath:public NxlString {
public:
	NxlPath() {}
	explicit NxlPath(const char *file):NxlString(file) {
	}
	explicit NxlPath(const wchar_t *file):NxlString(file) {
	}
	explicit NxlPath(const std::string &file):NxlString(file) {
	}
	explicit NxlPath(const std::wstring &file):NxlString(file) {
	}
	inline const TCHAR * tstr() const {
		return tchars();
	}
	inline const char *str() const {
		return string().c_str();
	}
	inline const wchar_t *wstr() const
	{
		return wstring().c_str();
	}
	inline const std::string &path() const {
		return string();
	}
	inline const std::wstring &wpath() const {
		return wstring();
	}
	inline const TCHAR *FindExtension() const {
		return PathFindExtension(tchars());
	}
	inline bool HasExtension(const TCHAR *ext) const {
		auto thisExt = FindExtension();
		if (ext == nullptr) return thisExt == nullptr;
		return thisExt != nullptr && _tcsicmp(ext, thisExt) == 0;
	}
#define NXL_EXT ".nxl"
	inline bool HasNxlExtension() const {
		return HasExtension(TEXT(NXL_EXT));
	}
	inline NxlPath EnsureNxlExtension() const {
		return HasNxlExtension() ? *this : NxlPath(_string + NXL_EXT, _wstring + WTEXT(NXL_EXT));
	}
	inline NxlPath RemoveNxlExtension() const {
		return HasNxlExtension() ? NxlPath(_string.substr(0, _string.length() - 4), _wstring.substr(0, _string.length() - 4)) : *this;
	}
	inline NxlPath Append(const std::string &ext) const
	{
		return NxlPath(_string + ext);
	}
	inline NxlPath Append(const std::wstring &ext) const
	{
		return NxlPath(_wstring + ext);
	}
	inline NxlPath AppendPath(const std::wstring &name) const {
		wchar_t *buffer = new wchar_t[_wstring.length() + name.length() + 5];
		wcscpy(buffer, _wstring.c_str());
		//if (!PathAppendW(buffer, name.c_str()))
		{
			wcscat(buffer, L"\\");
			wcscat(buffer, name.c_str());
		}
		auto path = NxlPath(buffer);
		delete[] buffer;
		return path;
	}
	inline NxlPath RemoveFileSpec() const {
		wchar_t buffer[MAX_PATH];
		wcscpy(buffer, _wstring.c_str());
		if (!PathRemoveFileSpecW(buffer)) {
			//TODO:
		}
		return NxlPath(buffer);
	}
	inline NxlPath WithoutExtension() const {
		const TCHAR * e = FindExtension();
		if (e != nullptr)
		{
			return NxlPath(tstring().substr(0, tstring().length() - _tcslen(e)));
		}
		else
		{
			return *this;
		}
	}
	inline bool IsValid() const {
		return !isempty();
	}
	inline bool CheckFileExists() const {
		DWORD attr = GetFileAttributesW(wstr());
		return attr != INVALID_FILE_ATTRIBUTES
			&& !(attr & FILE_ATTRIBUTE_DIRECTORY);
	}
	inline bool CheckExist() const {
		return GetFileAttributesW(wstr()) != INVALID_FILE_ATTRIBUTES;
	}
	bool AssertFileExists(const TCHAR*func, int line) const {
		DWORD e = GetFileAttributesW(wstr());
		if (e == INVALID_FILE_ATTRIBUTES)
		{
			DBGLOG("%s():FileNotFound:'%s'@func line-%d", func, tstr(), line);
			return false;
		}
		else if (e & FILE_ATTRIBUTE_DIRECTORY) {
			DBGLOG("%s():PathIsDirectory:'%s'@func line-%d", func, tstr(), line);
			return false;
		}
		return true;
	}
	NxlPath SearchFile(const std::wstring &name) const {
		auto path = AppendPath(name);
		if (path.CheckFileExists()) {
			return path;
		}
		return NxlPath();
	}
	static NxlPath NormalizeFilePath(const wchar_t *input) {
		NxlPath path(input);
		//if input is name only, append current directory
		if (path.wpath().find_last_of('\\') == std::wstring::npos)
		{
			//input paht only has file name
			return NxlPath::GetCurrectDirectory().AppendPath(path.wstr());
		}
		return path;
	}
	static NxlPath GetCurrectDirectory()
	{
		WCHAR buffer[MAX_PATH];
		if (GetCurrentDirectoryW(MAX_PATH, buffer))
		{
			return NxlPath(buffer);
		}
		else
		{
			ERRLOG("GetCurrentDirectory() failed-%lu(%#X)", GetLastError(), GetLastError());
		}
		return NxlPath();
	}
private:
	NxlPath(const std::string &path, const std::wstring &wpath) {
		_string = path;
		_wstring = wpath;
	}

};
