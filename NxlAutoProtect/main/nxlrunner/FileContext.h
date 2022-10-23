#pragma once
#include <stdafx.h>

class FileContext
{
public:
	static std::wstring GetLongPath(const std::wstring &filePath);
	explicit FileContext(const std::wstring &filePath) {
		_inputPath = filePath;
		const std::wstring fileName = PathFindFileName(_inputPath.c_str());
		_folder = _inputPath.substr(0, _inputPath.length() - fileName.length());

		ListFiles();

		if (g_rpmHelper->CheckFileStatus(_inputPath, &_isFileProtected, &_folderStatus)) {
			//check failed, treat it as non-proprty
			DBGLOG("Before:IsFileProtected=%d FolderStatus=%d", _isFileProtected, _folderStatus);
		}

		_originalExt = PathFindExtension(_inputPath.c_str());
		//if nxlrunner is started by proxy runner, the file can only be two types
		//1, file is protected and has .nxl extension
		//2, file is not protected and has .nxl extension
		if (_originalExt == L".nxl")
		{
			if (_isFileProtected)
			{
				//input is a normal nxl file
				_nxlPath = _inputPath;
			}
			else
			{
				_nxlPath.clear();
				//set rpm folder, and ask rpm to hide .nxl
				g_rpmHelper->AddRPMFolder(_folder);
				ListFiles();
			}
			_targetPath = _inputPath.substr(0, _inputPath.length() - _originalExt.length());
			//since the folder may not be RPM folder ,yet, GetLongPath() on target path will fail
			_targetLongPath = GetLongPath(_inputPath);
			_targetLongPath = _targetLongPath.substr(0, _targetLongPath.length() - _originalExt.length());
			_originalExt = PathFindExtension(_targetPath.c_str());
		}
		else
		{
			if (_isFileProtected)
			{
				//protect file has no .nxl extension
				_nxlPath = _inputPath + L".nxl";
				//append .nxl extension
				//int this case, nxlrunner is not started by proxyrunner, so don't need rename it back
				if (MoveFile(_inputPath.c_str(), _nxlPath.c_str()))
				{
					DBGLOG("Renamed '%s' to '%s'", _inputPath.c_str(), _nxlPath.c_str());
				}
				else
				{
					//TODO?
					DBGLOG("RenamedFailed(%lu)! '%s' to '%s'", GetLastError(), _inputPath.c_str(), _nxlPath.c_str());
				}
			}
			else
			{
				//normal file
				_nxlPath.clear();
			}
			_targetPath = _inputPath;
			_targetLongPath = GetLongPath(_targetPath);
		}
		DBGLOG("==>NxlFilePath:%s", _nxlPath.c_str());
		DBGLOG("==>TargetFilePath:%s", _targetPath.c_str());
		DBGLOG("==>TargetLongPath:%s", _targetLongPath.c_str());
		DBGLOG("==>TargetFileExtension:%s", _originalExt.c_str());

	}
	~FileContext() {
		//remove RPMDir
		//if old folder status already rpm dir, keep it
		if (_folderStatus != RPMHelper::IsRPMDir)
		{
			bool newFileStatus;
			RPMHelper::RPMFolderStatus newFolderStatus;
			if (g_rpmHelper->CheckFileStatus(_inputPath, &newFileStatus, &newFolderStatus)) {
				//check failed, treat it as non-proprty
				DBGLOG("Post:IsFileProtected=%d FolderStatus=%d", newFileStatus, newFolderStatus);
			}
			if (newFolderStatus == RPMHelper::IsRPMDir)
			{
				g_rpmHelper->RemoveRPMFolder(_folder);
			}
		}
		//TODO:what if lost protection
		ListFiles();
		DBGLOG("ends here");
	}
	void ListFiles() const
	{
		if (_folder.empty()) return;
		WIN32_FIND_DATA ffd;
		std::wstring pattern = _folder;
		if (pattern.back() != '\\')
		{
			pattern = pattern + L"\\*";
		}
		else
		{
			pattern = pattern + L"*";
		}
		HANDLE hFind = FindFirstFile(pattern.c_str(), &ffd);

		if (INVALID_HANDLE_VALUE == hFind)
			return;

		DBGLOG("Enumerating-'%s'...", pattern.c_str());
		do
		{
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (ffd.cFileName[0] != '.')
				{
					DBGLOG("\t%s <dir>", ffd.cFileName);
				}
			}
			else
			{
				DBGLOG("\t%s", ffd.cFileName);
			}
		} while (FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	inline const std::wstring &target_path() const {
		return _targetPath;
	}
	inline const std::wstring &target_long() const {
		return _targetLongPath;
	}
	inline const std::wstring &orig_ext() const {
		return _originalExt;
	}
	inline bool is_nxl_file() const {
		return !_nxlPath.empty();
	}
	inline const std::wstring &nxl_file() const {
		return _nxlPath;
	}
	inline const std::wstring &folder() const{
		return _folder;
	}
	bool GetFileLastWriteTime(__int64& nativeTime, __int64& nxlTime) const {
		if (is_nxl_file()) {
			nxlTime = GetLastWriteTime(nxl_file().c_str());
			nativeTime = 0;
			bool fileStatus;
			RPMHelper::RPMFolderStatus folderStatus;
			if (g_rpmHelper->CheckFileStatus(_inputPath, &fileStatus, &folderStatus)) {
				DBGLOG("IsFileProtected=%d FolderStatus=%d", fileStatus, folderStatus);
				if (folderStatus == RPMHelper::IsRPMDir
					|| folderStatus == RPMHelper::PosterityOfRPMDir) {
					//file is in RPM folder
					//grant trust to obtain time from native file
					if (g_rpmHelper->SetRMXStatus(true))
					{
						try
						{
							nativeTime = GetLastWriteTime(target_path().c_str());
						}
						catch (...)
						{
						}
							g_rpmHelper->SetRMXStatus(false);
					}
				}
			}
		}
		else
		{
			nativeTime = GetLastWriteTime(target_path().c_str());
			nxlTime = 0;
		}
		return true;
	}
	//disallow copy and assign
	FileContext(const FileContext &ctx) = delete;
	FileContext &operator=(const FileContext &ctx) = delete;
private:
	std::wstring _inputPath;
	std::wstring _targetPath;//without .nxl extension
	std::wstring _targetLongPath;//long path

	std::wstring _originalExt;
	std::wstring _nxlPath;

	bool _isFileProtected;
	std::wstring _folder;
	RPMHelper::RPMFolderStatus _folderStatus;
};
