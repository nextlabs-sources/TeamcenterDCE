// RPMUtils.Tests.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <functional>
#include <algorithm>
#include <RPMTest.h>

class BaseOperation
{
public:
	BaseOperation(const wchar_t *name, std::vector<std::wstring> argNames = {}) :_name(name), _argNames(argNames) {}
	virtual const std::wstring &GetName() { return _name; }
	virtual void PrintUsage() {
		std::wcout << GetName();
		for (auto a : _argNames)
		{
			std::wcout << " <" << a << ">";
		}
		std::wcout << std::endl;
	}
	virtual void Run(const std::vector<std::wstring> &args) = 0;
protected:
	std::wstring _name;
	std::vector<std::wstring> _argNames;
};
class BaseFuncCommand :public BaseOperation
{
public:
	BaseFuncCommand(const std::wstring &cmdName, std::function<bool()> func, std::vector<std::wstring> argNames = {}) :BaseOperation(cmdName.c_str(), argNames),_noArgFunc(func), _allArgsFunc(nullptr) {

	}
	BaseFuncCommand(const std::wstring &cmdName, std::function<bool(const std::vector<std::wstring> &)> func, std::vector<std::wstring> argNames = {}) :BaseOperation(cmdName.c_str(), argNames), _noArgFunc(nullptr), _allArgsFunc(func) {

	}
	virtual void Run(const std::vector<std::wstring> &args) override
	{
		bool ret;
		if (_noArgFunc) {
			ret = _noArgFunc();
		}
		else if (_allArgsFunc)
		{
			ret = _allArgsFunc(args);
		}
		else
		{
			PrintUsage();
			return;
		}
		if (ret)
		{
			std::wcout << "Success!" << std::endl;
		}
		else
		{
			std::wcout << "FAILED!" << std::endl;
		}
	}
protected:
	std::function<bool()> _noArgFunc;
	std::function<bool(const std::vector<std::wstring> &)> _allArgsFunc;
};
class CopyCommand:public BaseOperation
{
public:
	CopyCommand(const std::wstring &keyword) :BaseOperation(keyword.c_str(), {L"fromPath", L"toPath"}) {}
	void Run(const std::vector<std::wstring> &args) override
	{
		if (args.size() < 3) return;
		auto from = args[1];
		auto to = args[2];
		if (CopyFile(from.c_str(), to.c_str(), false))
		{
			std::cout << "==>Copied!" << std::endl;
		}
		else
		{
			std::cout << "==>FAILED!" << std::endl;
		}
	}
};
class OpenFileCommand :public BaseOperation
{
public:
	OpenFileCommand(const std::wstring &keyword) :BaseOperation(keyword.c_str(), {L"filePath"}) {}
	void Run(const std::vector<std::wstring> &args) override
	{
		if (args.size() < 2) return;
		auto file = args[1];
		SHELLEXECUTEINFO rSEI = { 0 };
		rSEI.cbSize = sizeof(rSEI);
		rSEI.lpVerb = L"open";
		rSEI.nShow = SW_NORMAL;
		rSEI.fMask = SEE_MASK_NOCLOSEPROCESS;
		rSEI.lpFile = file.c_str();
		rSEI.lpParameters = NULL;

		BOOL bExec = ShellExecuteEx(&rSEI);   // you should check for an error here
	}
};
class TestFileCommand :public BaseOperation
{
public:
	TestFileCommand(const std::wstring &keyword) :BaseOperation(keyword.c_str(), { L"filePath" }) {}
	void Run(const std::vector<std::wstring> &args) override
	{
		if (args.size() < 2) return;
		auto file = args[1];
		if (GetFileAttributes(file.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			std::cout << "==>Exists!" << std::endl;
		}
		else
		{
			std::cout << "==>FileNotFound!" << std::endl;
		}
	}
};
typedef std::function<bool(RPMSession*, const std::wstring&)> OneArgFunction;
typedef std::function<bool(RPMSession*, const std::vector<std::wstring>&)> AllArgsFunction;
class UtilsTestCommand :public BaseOperation
{
public:
	UtilsTestCommand(const std::wstring &keyword, RPMSession *utils, OneArgFunction func, const std::wstring argName=L"arg1")
		:BaseOperation(keyword.c_str(), {argName}), _utils(utils), _funcOne(func), _funcAll(nullptr) {
	}
	UtilsTestCommand(const std::wstring &keyword, RPMSession *utils, AllArgsFunction func, const std::vector<std::wstring> argNames = {}) 
		:BaseOperation(keyword.c_str(), argNames), _utils(utils), _funcOne(nullptr), _funcAll(func) {}

	void PrintUsage() override
	{
		std::wcout << GetName();
		if (_funcOne != nullptr)
		{
			std::wcout << " <" << _argNames.front() << ">";
		}
		else
		{
			for (auto a : _argNames)
			{
				std::wcout << " <" << a << ">";
			}
		}
		std::wcout << std::endl;
	}
	void Run(const std::vector<std::wstring> &args) override
	{
		if (RPMRun(args))
			return;
		const ResultInfo &error = _utils->GetLastError();
		if ( error== 0)
		{
			std::wcout <<GetName() << " success!" << std::endl;
		}
		else
		{
			std::wcout << GetName() << " failed!" << error.Message();
			if (error.InnerError() != nullptr) {
				std::wcout << "inner error='" << error.InnerError()->Message() << "'";
			}
			std::wcout<< std::endl;
			//std::vector<char> v(_utils->GetLastError().Message());
			std::wcout << "==>FAILED!" << std::endl;
		}
	}
	virtual bool RPMRun(const std::vector<std::wstring> &args) {
		if (_funcOne != nullptr)
		{
			if (args.size() < 2 || args[1].empty()) {
				std::wcout << GetName() << " <arg>" << std::endl;
				return true;
			}
			return _funcOne(_utils, args[1]);
		}
		else if(_funcAll != nullptr)
		{
			return _funcAll(_utils, args);
		}
		return false;
	}
protected:
	UtilsTestCommand(const std::wstring &keyword, RPMSession *utils) :BaseOperation(keyword.c_str()), _utils(utils), _funcOne(nullptr), _funcAll(nullptr) {}
	RPMSession *_utils;
	OneArgFunction _funcOne;
	AllArgsFunction _funcAll;
};
std::string wchars_to_chars(const std::wstring &wchars)
{
	std::stringstream ss;
	for (std::size_t j = 0; j < wchars.length(); j++)
	{
		ss << (char)(wchars.at(j));
	}
	return ss.str();
}
class ProtectCommand :public UtilsTestCommand
{
public:
	ProtectCommand(const std::wstring &keyword, RPMSession *utils, const std::vector<std::wstring> defaultTags = {}) :UtilsTestCommand(keyword, utils), _defaultTags(defaultTags) {}
	void PrintUsage() override 
	{
		std::wcout << GetName() << " <filePath> [[<key1> <val1>] [<key2> <val2>] ...]" << std::endl;
	}
	bool RPMRun(const std::vector<std::wstring> &args) override
	{
		if (args.size() < 2) {
			PrintUsage();
			return true;
		}
		auto file = args[1];
		NxlMetadataCollection tags;
		if (!_defaultTags.empty())
		{
			for (std::size_t i = 0; i < _defaultTags.size() - 1; i += 2)
			{
				auto k = wchars_to_chars(_defaultTags.at(i));
				auto v = wchars_to_chars(_defaultTags.at(i + 1));
				tags.Add(k.c_str(), v.c_str());
			}
		}
		for (std::size_t i = 2; i < args.size() - 1; i += 2)
		{
			auto k = wchars_to_chars(args.at(i));
			auto v = wchars_to_chars(args.at(i + 1));
			tags.Add(k.c_str(), v.c_str());
		}
		return _utils->ProtectFile(file.c_str(), tags, true) == 0;
	}
private:
	std::vector<std::wstring> _defaultTags;
};
bool AddRPMFolder(RPMSession *_utils, const std::wstring &arg) {
	bool isNew;
	_utils->AddRPMFolder(arg.c_str(), &isNew);
	if (isNew) {
		std::wcout << "New RPM folder is added" << std::endl;
	}
	else
	{
		std::wcout << "Folder is already RPM folder" << std::endl;
	}
	return false;
}
bool RemoveRPMFolder(RPMSession *_utils, const std::wstring &arg) {
	_utils->RemoveRPMFolder(arg.c_str());
	return false;
}
bool RegisterApp(RPMSession *_utils, const std::wstring &arg) {
	_utils->RegisterApp(true, arg.c_str());
	return false;
}
bool RegisterSelf(RPMSession *_utils, const std::wstring &arg) {
	int i = std::stoi(arg);
	_utils->RegisterApp(i > 0, L"");
	return false;
}
bool UnregisterApp(RPMSession *_utils, const std::wstring &arg) {
	_utils->RegisterApp(false, arg.c_str());
	return false;
}
bool DebugRPMDir(RPMSession *_utils, const std::wstring &arg) {
	wchar_t *file = new wchar_t[arg.length() + 1];
	int i = 0;
	for (auto c : arg)
	{
		file[i++] = c;
	}
	file[i] = '\0';
	RPMSession::RPMFolderStatus folderStatus;
	while (wcslen(file) > 3)
	{
		_utils->CheckFolderStatus(file, &folderStatus);
		std::wcout << file << ":" << folderStatus << std::endl;
		PathRemoveFileSpec(file);
	};
	delete[] file;
	return true;
}
bool ListRPMDir(RPMSession *_utils, const std::wstring &dir) {
	RPMSession::RPMFolderStatus status;
	std::vector<std::wstring> list;
	list.push_back(dir);
	int nrpmDir = 0;
	while (list.size() > 0)
	{
		auto d = list.back();
		list.pop_back();
		if (_utils->CheckFolderStatus(d.c_str(), &status) == 0)
		{
			//search sub folders
			WIN32_FIND_DATA ffd;
			HANDLE hFind = INVALID_HANDLE_VALUE;
			auto pattern = d + L"\\*";
			switch (status)
			{
			case RPMSession::IsRPMDir:
				std::wcout << d << std::endl;
				nrpmDir++;
			case RPMSession::AncestorOfRPMDir:
				hFind = FindFirstFile(pattern.c_str(), &ffd);

				if (INVALID_HANDLE_VALUE == hFind)
					break;

				do
				{
					if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
						&& ffd.cFileName[0] != '.')
					{
						list.push_back(d + L"\\" + ffd.cFileName);
					}
				} while (FindNextFile(hFind, &ffd) != 0);
				FindClose(hFind);
				break;
			default:
				break;
			}
		}
	}
	std::wcout << "Found " << nrpmDir << " RPM directory" << std::endl;
	return true;
}
bool ReadFileTags(RPMSession *_utils, const std::wstring &file, int mode)
{
	bool isProtected;
	NxlMetadataCollection tags;
	bool ret;
	switch (mode) {
	case 1:
		ret = _utils->RPMReadFileTags(file.c_str(), &isProtected, tags) == 0;
		break;
	case 2:
		ret = _utils->HelperReadFileTags(file.c_str(), &isProtected, tags) == 0;
		break;
	case 0:
	default:
		ret= _utils->ReadFileTags(file.c_str(), &isProtected, tags) == 0;
		break;
	}
	if (ret)
	{
		if (!isProtected)
		{
			std::wcout << "==>Not Protected file" << std::endl;
		}
		int ntags = tags.Count();
		std::wcout << "==>nTags=" << ntags << std::endl;
		for (int i = 0; i < ntags; i++)
		{
			const char *key, *val;
			if (tags.at(i, &key, &val))
			{
				std::wcout << "==>[" << i + 1 << "/" << ntags << "]:'" << key << "'='" << val << "'" << std::endl;
			}
		}
		return true;
	}
	return false;
}

bool IsFileProtected(RPMSession *_utils, const std::wstring &file)
{
	bool isprotected;
	if (_utils->CheckFileProtection(file.c_str(), &isprotected) == 0)
	{
		if (isprotected)
		{
			std::wcout << "'" << file << "' is protected" << std::endl;
		}
		else
		{
			std::wcout << "'" << file << "' is NOT protected" << std::endl;
		}
		return true;
	}
	return false;
}
bool AddTrustedProcess(RPMSession *_utils, const std::wstring &file)
{
	DWORD pid = std::stoul(file);
	_utils->SetTrustedProcess(pid, true);
	return false;
}
bool RemoveTrustedProcess(RPMSession *_utils, const std::wstring &file)
{
	DWORD pid = std::stoul(file);
	_utils->SetTrustedProcess(pid, false);
	return false;
}
bool SetRMXStatus(RPMSession *_utils, const std::wstring &file)
{
	int i = std::stoi(file);
	_utils->SetRMXStatus(i > 0);
	return false;
}
bool IsAppRegistered(RPMSession *_utils, const std::wstring &file)
{
	bool registered;
	if (_utils->IsAppRegistered(file.c_str(), &registered)==0) {
		std::wcout << file << ":register=" << registered << std::endl;
		return true;
	}
	return false;
}

int wmain(int argc, wchar_t * argv[])
{
	std::vector<BaseOperation*> map;
	RPMSession *utils = new RPMSession("{6829b159-b9bb-42fc-af19-4a6af3c9fcf6}");
	map.push_back(new CopyCommand(L"file-copy"));
	map.push_back(new BaseFuncCommand(L"file-move", [](const std::vector<std::wstring>&args) {
		if (args.size() < 3) { return false; }
		return MoveFileW(args[1].c_str(), args[2].c_str()) != 0; }));
	map.push_back(new TestFileCommand(L"file-exists"));
	map.push_back(new OpenFileCommand(L"file-open"));
	map.push_back(new UtilsTestCommand(L"rpm-logout", utils, [](RPMSession *u, const std::vector<std::wstring>& args) {return u->Logout() == 0; }));
	map.push_back(new UtilsTestCommand(L"rpm-list-policies", utils, [](RPMSession *u, const std::vector<std::wstring>& args) {return u->ListPolicies() == 0; }));
	map.push_back(new UtilsTestCommand(L"file-get-rights", utils, [](RPMSession *u, const std::wstring & file) {return u->GetFileRights(file.c_str()) == 0; }, L"file"));
	map.push_back(new UtilsTestCommand(L"rmx-status", utils, SetRMXStatus, L"0 or 1"));
	map.push_back(new UtilsTestCommand(L"trust-process-add", utils, AddTrustedProcess, L"pid"));
	map.push_back(new UtilsTestCommand(L"trust-process-remove", utils, RemoveTrustedProcess, L"pid"));
	map.push_back(new ProtectCommand(L"file-protect", utils));
	map.push_back(new ProtectCommand(L"file-protect-secret", utils, {L"ip_classification", L"secret"}));
	map.push_back(new UtilsTestCommand(L"file-get-tags", utils, [](RPMSession *u, const std::wstring & file) {return ReadFileTags(u, file, 0); }, L"file"));
	map.push_back(new UtilsTestCommand(L"file-get-tags-rpm", utils, [](RPMSession *u, const std::wstring & file) {return ReadFileTags(u, file, 1); }, L"file"));
	map.push_back(new UtilsTestCommand(L"file-get-tags-helper", utils, [](RPMSession *u, const std::wstring & file) {return ReadFileTags(u, file, 2); }, L"file"));
	map.push_back(new UtilsTestCommand(L"file-status", utils, [](RPMSession *u, const std::wstring & file) {
		bool isprotected;
		RPMSession::RPMFolderStatus status;
		if (u->CheckFileStatus(file.c_str(), &isprotected, &status) == 0)
		{
			std::wcout << "IsProtected=" << isprotected << " FolderStatus=" << status << std::endl;
			return true;
		}
		return false;
	}, L"file"));
	map.push_back(new UtilsTestCommand(L"file-is-protected", utils,IsFileProtected, L"file"));
	map.push_back(new UtilsTestCommand(L"rpm-dir-add", utils, AddRPMFolder, L"folder"));
	map.push_back(new UtilsTestCommand(L"rpm-dir-remove", utils, RemoveRPMFolder, L"folder"));
	map.push_back(new UtilsTestCommand(L"rpm-dir-debug", utils, DebugRPMDir, L"folder"));
	map.push_back(new UtilsTestCommand(L"rpm-dir-list", utils, ListRPMDir, L"folder"));
	map.push_back(new UtilsTestCommand(L"app-is-registered", utils, IsAppRegistered, L"appPath"));
	map.push_back(new UtilsTestCommand(L"app-register", utils, RegisterApp, L"appPath"));
	map.push_back(new UtilsTestCommand(L"app-unregister", utils, UnregisterApp, L"appPath"));
	map.push_back(new UtilsTestCommand(L"app-self-register", utils, RegisterSelf, L"0/1"));
	map.push_back(new UtilsTestCommand(L"file-ext-validate", utils, [](RPMSession *u, const std::wstring & file) {return u->ValidateFileExtension(file.c_str()) == 0; }, L"file"));
	map.push_back(new BaseFuncCommand(L"rpm-edit-copy", [](const std::vector<std::wstring>&args)
	{
		if (args.size() < 2)
			return false;
		auto file = args[1];
		std::wstring tarfile;
		if (args.size() > 2) {
			tarfile = args[2];
		}
		RPMTester tester;
		if (!tester.initialized())
			return false;
		if (!RPM_CALL(tester.Instance()->RPMEditCopyFile(file, tarfile)))
			return false;
		std::wcout << "'" << file << "' is copied to '" << tarfile << "'" << std::endl;
		unsigned int folderStatus;
		bool fileStatus;
		if (!RPM_CALL(tester.Instance()->RPMGetFileStatus(tarfile, &folderStatus, &fileStatus)))
			return false;
		std::wcout << "FolderStatus=" << folderStatus << " FileStatus=" << fileStatus << std::endl;
		return true;
	}, { L"nxlfile" }));
	
	if (argc > 1) {
		//command input from command line
		const std::wstring op = argv[1];
		for (auto o : map)
		{
			if (o->GetName() == op)
			{
				std::vector<std::wstring> opArgs;
				for (int i = 1; i < argc; i++) {
					opArgs.push_back(argv[i]);
				}
				o->Run(opArgs);
				return 0;
			}
		}
	}
	std::sort(map.begin(), map.end(), [](BaseOperation *cmd1, BaseOperation *cmd2) {return cmd1->GetName() < cmd2->GetName(); });
	while (true)
	{
		std::cout << std::endl << "===========================" << std::endl;
		std::cout << "Please input one of following commands:" << std::endl;
		for (auto o : map)
		{
			std::wcout << "\t";
			o->PrintUsage();
		}
		std::wstring line;
		if (!std::getline(std::wcin, line) || line.length() <= 0) continue;
		bool inquote = false;
		std::size_t i = 0;
		std::vector<std::wstring> args;
		for (std::size_t j = 0; j <= line.length(); j++)
		{
			wchar_t c;
			if (j == line.length() || (c = line.at(j)) == ' ' || c == '\"')
			{
				if (c == ' ' && inquote) continue;
				//split
				if (j > i)
				{
					args.push_back(std::wstring(line, i, j - i));
				}
				else if (j == i&&c == '\"'&&inquote) 
				{
					args.push_back(std::wstring());
				}
				//next section
				i = j + 1;
				if(c=='\"')
					inquote = !inquote;
			}
		}
		for (auto o : map)
		{
			if (o->GetName() == args[0])
			{
				std::wcout << L"Running..." << std::endl;
				for (auto a : args) {
					std::wcout << "\t" << a << std::endl;
				}
				o->Run(args);
				system("pause");
				break;
			}
		}
	}
    return 0;
}

