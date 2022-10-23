#pragma once
#include <stdafx.h>
#include <sstream>
#include <fstream>
#include <string>
#include <utils_system.h>
#include <cerrno>
#include <NxlUtils/string_utils.hpp>
#include <translator.h>
#ifndef WNT
#include <dirent.h> //opendir readdir
#endif
#include <NxlUtils/TonxlfileTranslator.hpp>

#pragma warning(disable : 4996)

typedef std::tuple<std::string, std::string, std::string> dataset_info_t;

class ImportContext {
public:
	ImportContext() {
		auto cmdArgs = load_cmd_args();
		for (auto arg = cmdArgs.begin(); arg != cmdArgs.end(); arg++) {
			size_t ieq = arg->rfind('=');
			if (ieq != std::wstring::npos)
			{
				auto key = trim_copy(arg->substr(0, ieq));
				auto val = trim_copy(arg->substr(ieq + 1));
				if (iequals(key, "-assem")
					|| iequals(key, "-assembly"))
				{
					LoadAssembly(dequote(val));
				}
				else if (iequals(key, "-l")
					|| iequals(key, "-load_log_file"))
				{
					LoadLogFile(dequote(val));
				}
				else if (iequals(key, "-default_a")
					|| iequals(key, "-default_action"))
				{
					_defaultAction = val;
				}
			}
		}
	}
	const std::string &working_dir() {
		if (_workingDir.empty()) {
			std::string tempDir;
			char *ugiitmpdir = getenv("UGII_TMP_DIR");
			DBGLOG("UGII_TMP_DIR='%s'...", ugiitmpdir);
			if (!ugiitmpdir) {
				char tmpdir[MAX_PATH];
				if (GetTempPath(MAX_PATH, tmpdir)) {
					tempDir = tmpdir;
				}
			}
			std::stringstream ss;
#ifdef WNT
			ss << GetCurrentProcessId();
#else
			ss << getpid();
#endif
			DBGLOG("temp='%s' projectName='%s' pid='%s'", tempDir.c_str(), PROJECT_NAME, ss.str().c_str());
			_workingDir = BuildPath(tempDir, std::string(PROJECT_NAME), std::string(ss.str()));
			DBGLOG("WorkingDir='%s'", _workingDir.c_str());
		}
		return _workingDir;
	}
	void pm_log(const std::string &name) {
		file_exists(BuildPath(working_dir(), name));
	}
	void OnImportBuilderInitialized(tag_t importBuilderTag)
	{
		_importBuilder = importBuilderTag;
		std::string stageFolder = BuildPath(working_dir(), std::string("translator"));
		if (!dir_exists(stageFolder)) {
			mkpath(stageFolder);
		}
		std::string backupFolder = BuildPath(working_dir(), std::string("backup"));
		if (!dir_exists(backupFolder)) {
			mkpath(backupFolder);
		}
		for (auto file = _associatedFiles.begin(); file != _associatedFiles.end(); file++)
		{
			std::string outputFile = stageFolder;
			if (_backupFiles.find(*file) != _backupFiles.end())
				continue;
			if (TonxlfileTranslator::unprotect(*file, outputFile))
			{
				//swap file
				std::string backupFile = BuildPath(backupFolder, FindFileNameInPath(*file));
				DBGLOG("Backup '%s' as '%s'", file->c_str(), backupFile.c_str());
				if (std::rename(file->c_str(), backupFile.c_str()) < 0)
				{
					NXERR("=>failed('%s')", strerror(errno));
					continue;
				}
				_backupFiles.insert(std::make_pair(*file, backupFile));
				DBGLOG("Retrive decrypted file '%s'", outputFile.c_str());
				if (std::rename(outputFile.c_str(), RemoveNxlExtension(*file).c_str()) < 0)
				{
					NXERR("=>failed('%s')", strerror(errno));
				}
			}
		}
	}
	void OnImportBuilderTerminating()
	{
		//clean up unprotected data
		DBGLOG("Restoring %zu backup files...", _backupFiles.size());
		size_t i = 0;
		for (auto e = _backupFiles.begin(); e != _backupFiles.end(); e++) {
			const std::string &origFile = e->first;
			std::string &backupFile = e->second;
			std::string unprotectedFile = RemoveNxlExtension(origFile);
			DBGLOG("[%zu/%zu]OrigFile='%s' BackupFile='%s'", i + 1, _backupFiles.size(), origFile.c_str(), backupFile.c_str());
			if (file_exists(unprotectedFile)) {
				DBGLOG("Deleting unproteced file-'%s'", unprotectedFile.c_str());
				if (std::remove(unprotectedFile.c_str()) < 0)
				{
					NXERR("=>Delete failed('%s')", strerror(errno));
				}
			}
			else
			{
				DBGLOG("File-'%s' doesn't exist", unprotectedFile.c_str());
			}
			if (file_exists(backupFile))
			{
				DBGLOG("Restoring backup file-'%s' to '%s'", backupFile.c_str(), origFile.c_str());
				if (std::rename(backupFile.c_str(), origFile.c_str()) < 0)
				{
					NXERR("=>Restore failed('%s')", strerror(errno));
				}
			}
			else
			{
				NXERR("Faile to locate backup file-'%s'", backupFile.c_str());
			}
		}
	}
	void OnImportBuilderFinalized()
	{
		////updating Dataset infos
		//DBGLOG("Updating Datasets...");
		//for (auto dsInfo = _datasetInfos.begin(); dsInfo != _datasetInfos.end(); dsInfo++) {
		//	teamcenter_protect(std::get<0>(*dsInfo).c_str(), std::get<1>(*dsInfo).c_str(), std::get<2>(*dsInfo).c_str(), "");
		//}
	}
	inline const std::vector<std::string> &associatedFiles() const { return _associatedFiles; }

	inline void OnPostCommit(const std::string &fileName, dataset_info_t &&dsInfo) {
		auto hasbackup = _backupFiles.find(fileName);
		if (hasbackup != _backupFiles.end())
		{
			DBGLOG("File-'%s' is protected, force protect the dataset", fileName.c_str());
			teamcenter_protect(std::get<0>(dsInfo).c_str(), std::get<1>(dsInfo).c_str(), std::get<2>(dsInfo).c_str(), "");
		}
	}
	inline tag_t ImportBuilder() const { return _importBuilder; }
private:
	void LoadAssembly(const std::string &file) {
		_topAssembly = file;
		LoadAssociateDir(FindDirectoryInPath(file));
	}
	void LoadAssociateDir(const std::string &dir) {
		std::vector<std::string> subdirs;
#ifdef WNT
		WIN32_FIND_DATA ffd;
		auto pattern = BuildPath(dir, std::string("*"));
		HANDLE hFind = FindFirstFile(pattern.c_str(), &ffd);

		if (INVALID_HANDLE_VALUE == hFind)
			return ;
		DBGLOG("Enumerating-'%s'...", pattern.c_str());
		do
		{
			auto path = BuildPath(dir, std::string(ffd.cFileName));
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (ffd.cFileName[0] != '.')
				{
					DBGLOG("\t%s <dir>", ffd.cFileName);
					subdirs.push_back(path);
				}
			}
			else
			{
				DBGLOG("\t%s", ffd.cFileName);
				_associatedFiles.push_back(path);
			}
		} while (FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
#else
		DIR *pDir;
		struct dirent *pDirent;
		if (pDir = opendir(dir.c_str())) {
			while ((pDirent = readdir(pDir)) != NULL) {
				auto path = BuildPath(dir, std::string(pDirent->d_name));
				switch (pDirent->d_type)
				{
				case DT_DIR:
					if (pDirent->d_name[0] != '.')
					{
						DBGLOG("\t%s <dir>", pDirent->d_name);
						subdirs.push_back(path);
					}
					break;
				case DT_REG:
					DBGLOG("%s <reg>", pDirent->d_name);
					_associatedFiles.push_back(path);
					break;
				case DT_UNKNOWN:
				default:
					DBGLOG("%s <%d>", pDirent->d_name, pDirent->d_type);
					_associatedFiles.push_back(path);
					break;
				}
			}
			closedir(pDir);
		}
#endif
		for (auto subdir = subdirs.begin(); subdir != subdirs.end();subdir++) {
			LoadAssociateDir(*subdir);
		}
	}
	void LoadLogFile(const std::string &logFile) {
		std::string line;
		std::ifstream infile(logFile.c_str());
		while (!(std::getline(infile, line).fail()))
		{
			std::istringstream iss(line);
			std::string prefix;
			iss >> prefix;
			if (iequals(prefix, "&LOG"))
			{
				std::string key, value;
				iss >> key >> value;
				if (iequals(key, "TOP_ASSEMBLY"))
				{
					iss >> value;
					_topAssembly = dequote(value);
					_associatedFiles.push_back(_topAssembly);
					DBGLOG("TOP_ASSEMBLY:%s", _associatedFiles.back().c_str());
				}
				else if (iequals(key, "Part:")) {
					_associatedFiles.push_back(dequote(value));
					DBGLOG("Part:%s", _associatedFiles.back().c_str());
				}
			}
		}
	}
	tag_t _importBuilder;
	std::string _workingDir;
	std::string _defaultAction;//
	std::string _topAssembly;
	std::vector<std::string> _associatedFiles;
	//std::vector<std::pair<std::string, std::string>> _backupFiles;
	std::unordered_map<std::string, std::string> _backupFiles;
	//std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> _fileTags;
	//std::vector<dataset_info_t> _datasetInfos;
};
extern ImportContext *s_pImportContext;
#ifdef DEBUG
#define PMLOG() s_pImportContext->pm_log(__FUNCTION__)
#define PMLOG2(arg) s_pImportContext->pm_log(BuildPath(std::string(__FUNCTION__) , std::string(arg)))
#define PMLOG3(arg1, arg2) s_pImportContext->pm_log(BuildPath(std::string(__FUNCTION__) , std::string(arg1), std::string(arg2)))
#else
#define PMLOG()
#define PMLOG2(arg)
#define PMLOG3(arg1, arg2)
#endif
