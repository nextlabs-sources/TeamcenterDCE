#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <set>

#include "..\common\CommonTypes.h"

#include <pfcCommand.h>
#include <pfcModel.h>
#include <wfcClient.h>
#include <OtkXModel.h>

#if defined(XTEST_EXPORTS)
#define XTESTAPI __declspec(dllexport)
#else
#define XTESTAPI __declspec(dllimport)
#endif

#if defined(__cplusplus)
extern "C" {
#endif
XTESTAPI bool IsTestCaseRunning();
XTESTAPI void NotifyTestFile(const wchar_t* testNxlFile, const wchar_t* nxlOrigin, const char* tags);
XTESTAPI void DumpCachedNxlModels(const std::map<std::wstring, std::wstring>& mdls);
#if defined(__cplusplus)
}
#endif
extern "C" {
	typedef wfcStatus(*TestCommandCallback) ();
}

struct TestCommand
{
	string m_cmdName;
	TestCommandCallback m_runCallback;
	TestCommandCallback m_verifyCallback;
	string m_cmdLabel;
	TestCommand() : m_runCallback(nullptr), m_verifyCallback(nullptr) {}
};

class TestCommandListener : public virtual pfcUICommandActionListener
{
public:
	TestCommandListener(const std::string& cmd) : m_cmdName(cmd) {};
	void OnCommand();
private:
	std::string m_cmdName;
};

class TestCommandBracketListener : public virtual pfcUICommandBracketListener
{
public:
	TestCommandBracketListener(const std::string& cmd) : m_cmdName(cmd) {};
	void OnBeforeCommand();
	void OnAfterCommand();
private:
	std::string m_cmdName;
};

typedef std::shared_ptr<COtkXCacheModel> TCacheModelPtr;

class CTestFwk
{
	RMX_SINGLETON_DECLARE(CTestFwk)
public:
	bool Init();
	bool InitWorkFolder();
	bool SetupTestCases();
	void ResetTestCaseEnv();
	
	wfcStatus RunTestCase(const std::string& scriptFile);
	void CompleteTestCase();
	
	void ReportTestResult();

	void ReopenTestModel();

	void OnTestCommand(std::string& cmdName);
	void OnBeforeTestCommand(std::string& cmdName);
	void OnAfterTestCommand(std::string& cmdName);

	wfcStatus ViewInfo();
	wfcStatus ViewModelInfo();
	bool IsTestCaseRunning();

public:
	std::wstring GetOutputDir() const {
		return m_testOutputRoot; 
	}

	std::wstring GetOutputRPMDir() const { return m_testOutputRoot_RPM;}

	std::wstring GetOutputNonRPMDir() const { return m_testOutputRoot_NonRPM; }

	std::wstring GetInstallDir();

	std::wstring GetInstallScriptsDir()  const { return m_fwkScriptsDir; }

	std::wstring GetTestLogError()
	{
		const static std::wstring logDir = m_testOutputRoot + L"\\XTestError.log";
		return logDir;
	}

	std::wstring GetTestLogInfo()
	{
		const static std::wstring logDir = m_testOutputRoot + L"\\XTest.log";
		return logDir;
	}

	std::wstring GetTestModel() const { return m_srcMdlToTest.GetOrigin(); }
	std::wstring GetTestWorkDir() const { return m_srcWorkDir; }

	void AddTestFileInfo(const std::wstring& testNxlFile, const std::wstring& nxlOrigin, const std::string& tags);
	std::pair<wstring, string> FindNxlOrigin(const std::wstring& testNxlFile) const;
	TCacheModelPtr FindCacheModel(const std::wstring& filePath);
	void UpdateTestFileInfo(const std::wstring& origTestFile, const std::wstring& newTestFile);

public:
	wfcStatus VerifyCurrentModel();
	wfcStatus VerifyFile(const std::wstring& filePath);
	wfcStatus VerifyAllOutputDirs();
	wfcStatus VerifySelection();

private:
	bool VerifyFolder(const std::wstring & dir);
	wfcStatus VerifyParentFile(const std::wstring& asmFilePath);
	wfcStatus VerifyDepNxlFiles(pfcModel_ptr pParentMdl, set<wstring>& nxlFiles);
	wfcStatus VerifyNxlFileTags(const std::wstring& filePath);
	wfcStatus VerifySingleFile(const std::wstring& filePath);

	TestCommand FindTestCommand(const std::string& cmdName)
	{
		for (const auto& cmd : m_testCommands)
		{
			if (cmd.m_cmdName == cmdName)
			{
				return cmd;
			}
		}

		return TestCommand();
	}

	void ResetWorkDir();
	pfcModel_ptr GetSelectedModel();

private:
	std::wstring m_fwkInstallDir;
	std::wstring m_fwkScriptsDir;

	std::wstring m_testOutputRoot;
	std::wstring m_testOutputRoot_RPM;
	std::wstring m_testOutputRoot_NonRPM;

	std::wstring m_srcWorkDir;
	COtkXCacheModel m_srcMdlToTest;
	
	std::vector<std::string> m_failedTestCases;
	std::string m_currTestCase;

	std::vector<TestCommand> m_testCommands;

	typedef std::map<std::wstring, std::pair<std::wstring, std::string>, ICaseKeyLess> TTestFileMap;
	TTestFileMap m_testFileInfo;
	TTestFileMap m_passedFiles;

	std::map<std::wstring, TCacheModelPtr, ICaseKeyLess> m_srcAllMdlToTest;
};

inline CTestFwk& TestFwk()
{
	return CTestFwk::GetInstance();
}