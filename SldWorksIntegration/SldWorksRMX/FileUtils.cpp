#include "FileUtils.h"
#include "CommonUtils.h"
#include "PathEx.h"

#define HAS_BIT(flags, mask) ((flags) & (mask)) == (mask)

namespace FileUtils {

	wstring GetFileName(wstring fileStr) {
		wchar_t fileName[_MAX_FNAME];
		const wchar_t * filePath = fileStr.c_str();
		int retCode = _wsplitpath_s(filePath, NULL, 0, NULL, 0, fileName, _MAX_FNAME, NULL, 0);
		if (retCode != 0) {
			LOG_ERROR("Error Code = " << retCode);
			return wstring();
		}
		return wstring(fileName);

	}


	wstring GetFileExt(wstring fileStr) {
		wchar_t fileExt[_MAX_EXT];
		const wchar_t * filePath = fileStr.c_str();
		int retCode = _wsplitpath_s(filePath, NULL, 0, NULL, 0, NULL, 0, fileExt, _MAX_EXT);
		if (retCode != 0) {
			LOG_ERROR("Error Code = " << retCode);
			return wstring();
		}
		return wstring(fileExt);
	}


	wstring GetFileDirectory(wstring fileStr) {
		wchar_t fileDir[_MAX_DIR];
		const wchar_t *filePath = fileStr.c_str();
		int retCode = _wsplitpath_s(filePath, NULL, 0, fileDir, _MAX_PATH, NULL, 0, NULL, 0);
		if (retCode != 0) {
			LOG_ERROR("Error Code = " << retCode);
			return wstring();
		}
		return wstring(fileDir);

	}

	std::wstring GetParentPath(wstring fileStr)
	{
		size_t pos = fileStr.find_last_of(L"\\");
		if (pos != std::wstring::npos)
			return fileStr.substr(0, pos);

		pos = fileStr.find_last_of(L"/");
		if (pos != std::wstring::npos)
			return fileStr.substr(0, pos);

		return std::wstring();
	}

	bool RenameFile(const wchar_t *oldname, const wchar_t *newname) {
		int errorno = _wrename(oldname, newname);
		switch (errorno) {
		case EACCES:
			LOG_ERROR("File or directory specified by newname already exists or could not be created (invalid path) \
			or oldname is a directory and newname specifies a different path.");
			return false;
		case ENOENT:
			LOG_ERROR("File or path specified by oldname not found.");
			return false;
		case EINVAL:
			LOG_ERROR("Name contains invalid characters.");
			return false;
			break;
		default:
			break;
		}

		return true;

	}

	bool DeleteFileUtil(LPCWSTR lpFileName) {
		BOOL err;
		err = DeleteFile(lpFileName);
		if (err == 0) {
			DWORD lasterror = GetLastError();
			LOG_ERROR_FMT(L"Delete file failed. FileName = %s , Error Code = %lu ", lpFileName, lasterror);
			return false;
		}
		return true;
	}

	bool ForceDelete(LPCWSTR lpFileName) {
		LOG_INFO("Attempt to force delete the file. FileName = " << lpFileName);
		BOOL err = DeleteFile(lpFileName);
		if (err == 0) {
			DWORD lasterror = GetLastError();
			if (lasterror == ERROR_ACCESS_DENIED) {
				//Check if the file is read-only
				bool isReadOnly = GetFileAttributeBit(lpFileName, FILE_ATTRIBUTE_READONLY);
				if (isReadOnly) {
					LOG_INFO("File is Read-Only. FileName = " << lpFileName);
					//Unset the read-only flag on the file
					FileUtils::UnsetFileAttributeBit(lpFileName, FILE_ATTRIBUTE_READONLY);
					err = DeleteFile(lpFileName);
					if (err) {
						LOG_INFO("Force Delete successful for read-only file. FileName = " << lpFileName);
						return true;
					}
					LOG_ERROR("Force deleting a read-only file failed. FileName = " << lpFileName);
					//Set the read-only flag back on the file because the attempt to delete file failed.
					FileUtils::SetFileAttributeBit(lpFileName, FILE_ATTRIBUTE_READONLY);
				}
			}
			LOG_ERROR_FMT("Force Delete failed for file = %s due to error = %lu", lpFileName, GetLastError());
			return false;
		}
		return true;
	}


	bool CopyFileUtil(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName) {
		BOOL  bFailIfExists = TRUE;
		BOOL retVal = CopyFile(lpExistingFileName, lpNewFileName, bFailIfExists);
		if (retVal == 0) {
			LOG_ERROR_FMT("Error occured while copying file . ExistingFileName:%s , ToFileName:%s, Error Code:%d", lpExistingFileName, lpNewFileName, GetLastError());
			return false;
		}
		return true;
	}


	wstring GetFileNameWithExt(wstring fileStr) {
		return GetFileName(fileStr) + GetFileExt(fileStr);

	}

	bool IsFileExists(const std::wstring &filePath) {
		BOOL isExists = PathFileExists(filePath.c_str());
		if (isExists != 1) {
			return false;
		}
		return true;
	}

	bool IsExists(const std::wstring &path){
		if (path.empty())
			return false;

		struct _stat status;
		return ((_wstat(path.c_str(), &status) == 0));

	}


	bool IsFile(const std::wstring & path)
	{
		if (path.empty())
			return false;

		struct _stat status;
		return ((_wstat(path.c_str(), &status) == 0) && ((status.st_mode & S_IFMT) != 0));
	}

	bool IsDir(const std::wstring & path)
	{
		if (path.empty())
			return false;

		struct _stat status;
		return ((_wstat(path.c_str(), &status) == 0) && ((status.st_mode & S_IFDIR) != 0));
	}

	wstring GetAbsolutePath(const std::wstring & path) {
		wchar_t absPath[_MAX_PATH];
		if (_wfullpath(absPath, path.c_str(), _MAX_PATH) != NULL)
			LOG_DEBUG("GetAbsolutePath. Full path is = " << absPath);
		else
			LOG_ERROR("GetAbsolutePath.Invalid path\n");

		return absPath;

	}

	wstring bstr2ws(BSTR bs) {
		// given BSTR bs
		assert(bs != nullptr);
		std::wstring ws(bs, SysStringLen(bs));
		return ws;
	}

	wstring GetAvailableFileName(const wstring fileDir, const wstring pattern, const wstring fileExt) {
		int fileCopyCount = 0;
		wstring newFileName = fileDir + L"\\" + pattern + fileExt;
		while (FileUtils::IsFileExists(newFileName))
		{
			++fileCopyCount;
			newFileName = fileDir + L"\\" + pattern + to_wstring(fileCopyCount) + fileExt;
		}
		return newFileName;
	}



	bool GetFileAttributeBit(const wstring &fileName, DWORD bit) {
		bool isAttributePresent = false;
		DWORD attr = GetFileAttributes(fileName.c_str());
		if (attr != INVALID_FILE_ATTRIBUTES) {
			isAttributePresent = HAS_BIT(attr, bit);
		}

		return isAttributePresent;
	}

	bool SetFileAttributeBit(const wstring &fileName, DWORD bit) {
		bool status = false;
		DWORD attr = GetFileAttributes(fileName.c_str());
		if (attr != INVALID_FILE_ATTRIBUTES) {
			BOOL ret = SetFileAttributes(fileName.c_str(), attr | bit);
			if (!ret) {
				LOG_ERROR_FMT("Setting attribute bit = %lu on file = %s failed due to error = %lu",bit,fileName.c_str(),GetLastError());
				return false;
			}
			status = true;
		}

		return status;
	}


	bool UnsetFileAttributeBit(const wstring &fileName, DWORD bit) {
		bool status = false;
		DWORD attr = GetFileAttributes(fileName.c_str());
		if (attr != INVALID_FILE_ATTRIBUTES) {
			BOOL ret = SetFileAttributes(fileName.c_str(), attr & ~bit);
			if (!ret) {
				LOG_ERROR_FMT("Unsetting attribute bit = %lu on file = %s failed due to error = %lu", bit, fileName.c_str(), GetLastError());
				return false;
			}
			status = true;
		}

		return status;

	}

	void wsTokenizer(wstring ws, const wstring& delim,vector<wstring>& tokens) {
		wchar_t *token = nullptr;
		wchar_t *next_token = nullptr;
		wchar_t *buff = new wchar_t[ws.length()+1];
		wcscpy_s(buff,ws.length()+1,ws.c_str());
		token = wcstok_s(buff,delim.c_str(),&next_token);
		while (token != nullptr) {
			tokens.push_back(token);
			token = wcstok_s(NULL,delim.c_str(),&next_token);
		}

		delete [] buff;
	}

	wstring GetEnvVariablePath(const wstring& dir) {
		//Directory should be of form : ${USERPROFILE}\\Siemens\\swcache
		if (dir[0] != '$')
			return dir;

		size_t pos1 = dir.find_first_of(L"{");
		size_t pos2 = dir.find_first_of(L"}");
		size_t pos3 = dir.find_first_of(L"\\");

		wstring env_var = wstring(dir.begin() + pos1 + 1, dir.begin() + pos2);
		wstring env_var_val = RMXUtils::getEnv(CommonUtils::rtrim(CommonUtils::ltrim(env_var)));
		wstring absPath = env_var_val + wstring(dir.begin() + pos3, dir.end());
		return GetAbsolutePath(absPath);
	}

	bool CreateDirIfNotExists(const wstring &dir) {
		if (!IsFileExists(dir)) {
			CPathEx pathEx(dir.c_str());
			int ret = pathEx.CreateDirectories(dir);
			if (ret == ERROR_SUCCESS) {
				LOG_INFO("Created new directory. Directory = " << dir.c_str());
				return true;
			}
			return false;
		}
		return true;
	}

}



