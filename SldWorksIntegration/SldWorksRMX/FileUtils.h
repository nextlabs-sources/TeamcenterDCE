#pragma once

#include "stdafx.h"
namespace FileUtils {

	wstring GetFileName(wstring fileStr);
	wstring GetFileNameWithExt(wstring fileStr);
	wstring GetFileExt(wstring fileStr);
	wstring GetFileDirectory(wstring fileStr);
	wstring GetParentPath(wstring fileStr);
	bool RenameFile(const wchar_t *oldname, const wchar_t *newname);
	bool DeleteFileUtil(LPCWSTR lpFileName);
	bool ForceDelete(LPCWSTR lpFileName);
	bool CopyFileUtil(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName);
	bool IsFileExists(const wstring &filePath);
	bool IsExists(const std::wstring &path);
	bool IsDir(const std::wstring & path);
	bool IsFile(const std::wstring & path);
	wstring GetAbsolutePath(const std::wstring & path);
	wstring GetAvailableFileName(const wstring fileDir, const wstring pattern,const wstring fileExt);
	bool GetFileAttributeBit(const wstring &fileName,DWORD bit);
	bool SetFileAttributeBit(const wstring &fileName, DWORD bit);
	bool UnsetFileAttributeBit(const wstring &fileName, DWORD bit);
	wstring bstr2ws(BSTR bs);
	void wsTokenizer(wstring ws, const wstring &delim, vector<wstring>& tokens);
	wstring GetEnvVariablePath(const wstring& dir);
	bool CreateDirIfNotExists(const wstring &dir);
}; //namespace FileUtils