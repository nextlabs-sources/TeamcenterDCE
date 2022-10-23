#include "PathEx.h"

#include <direct.h>
#include <Shlobj.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <Windows.h>

CPathEx::CPathEx()
{
}

CPathEx::CPathEx(const wchar_t * path)
	: m_path(std::wstring(path))
{
}

CPathEx & CPathEx::operator=(const wchar_t * path)
{
	m_path = path;
	return *this;
}

std::wstring CPathEx::GetExtension() const
{
	wchar_t fileExt[_MAX_EXT];
	errno_t err = _wsplitpath_s(m_path.c_str(), NULL, 0, NULL, 0, NULL,
								0, fileExt, _MAX_EXT);

	if (err == 0)
		return std::wstring(fileExt);

	return std::wstring();
}

std::wstring CPathEx::GetParentPath() const
{
	size_t pos = m_path.find_last_of(PATH_DELIMITER_1);
	if (pos != std::wstring::npos)
		return m_path.substr(0, pos);

	pos = m_path.find_last_of(PATH_DELIMITER_2);
	if (pos != std::wstring::npos)
		return m_path.substr(0, pos);

	return std::wstring();
}

std::wstring CPathEx::GetFileName() const
{
	wchar_t fileName[_MAX_FNAME];
	errno_t err = _wsplitpath_s(m_path.c_str(), NULL, 0, NULL, 0, fileName,
								_MAX_FNAME, NULL, 0);

	if (err == 0)
		return std::wstring(fileName);

	return std::wstring();
}

std::wstring CPathEx::GetFileNameWithExt() const
{
	wchar_t fileExt[_MAX_EXT];
	wchar_t fileName[_MAX_FNAME];
	errno_t err = _wsplitpath_s(m_path.c_str(), NULL, 0, NULL, 0, fileName,
		_MAX_FNAME, fileExt, _MAX_EXT);

	if (err == 0)
		return std::wstring(fileName) + std::wstring(fileExt);

	return std::wstring();
}

CPathEx& CPathEx::operator/=(const wchar_t* path)
{
	if (m_path.back() != PATH_DELIMITER_1)
		m_path += PATH_DELIMITER_1;
		
	m_path += path;
	return *this;
}

CPathEx& CPathEx::operator+=(const wchar_t* path)
{
	m_path += path;
	return *this;
}

const wchar_t * CPathEx::c_str() const
{
	return m_path.c_str();
}

std::wstring CPathEx::ToString() const
{
	return m_path;
}

bool CPathEx::Exists(const std::wstring & path)
{
	if (path.empty())
		return false;

	return (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES);
}

bool CPathEx::IsFile(const std::wstring & path)
{
	if (path.empty())
		return false;
		
	DWORD attrs = ::GetFileAttributesW(path.c_str());
	if (attrs == INVALID_FILE_ATTRIBUTES)
		return false;

	return ((attrs & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY);
}

bool CPathEx::IsDir(const std::wstring & path)
{
	if (path.empty())
		return false;
		
	DWORD attrs = ::GetFileAttributesW(path.c_str());
	if (attrs == INVALID_FILE_ATTRIBUTES)
		return false;

	return ((attrs & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
}

bool CPathEx::RemoveFile(const std::wstring & path)
{
	// For read-only file, we shall reset the read-only attribute first, or else will
	// Access denied when delete it
	auto attrs = GetFileAttributesW(path.c_str());
	if ((INVALID_FILE_ATTRIBUTES != attrs) && (attrs & FILE_ATTRIBUTE_READONLY))
	{
		attrs &= ~FILE_ATTRIBUTE_READONLY;
		SetFileAttributes(path.c_str(), attrs);
	}

	return (_wremove(path.c_str()) == 0);
}

int CPathEx::CreateDirectories(const std::wstring & path)
{
	wchar_t bufferDir[_MAX_DIR];
	if (GetFullPathNameW(path.c_str(), _MAX_DIR, &bufferDir[0], nullptr) == 0)
		return GetLastError();

	return SHCreateDirectory(nullptr, &bufferDir[0]);
}

int CPathEx::RenameFile(const std::wstring & origFilePath, const std::wstring & newFilePath)
{
	if (_wrename(origFilePath.c_str(), newFilePath.c_str()) == 0)
		return ERROR_SUCCESS;

	switch (errno)
	{
	case EACCES:
		return ERROR_ACCESS_DENIED;
	case ENOENT:
		return ERROR_FILE_NOT_FOUND;
	case EINVAL:
		return ERROR_INVALID_NAME;
	default:
		return ERROR_SUCCESS;
	}
}
