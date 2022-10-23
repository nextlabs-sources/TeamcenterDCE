#pragma once

#include <string>
#include <vector>
#include <set>

#include <pfcCommand.h>
#include <wfcClient.h>
#include <pfcModel.h>

#define MSG_FILE "msg_user.txt"
#define RUN_TEST(func)  TestFwk().RunTestCase(_STRINGIZE(Test_##func##.txt))

#define DECLAR_TESTFUNC(func) \
extern "C" wfcStatus XTest_##func##_Run(); \
extern "C" wfcStatus  XTest_##func##_Verify();

#define DECLAR_HLP_FUNC(func) \
extern "C" wfcStatus XTest_##func##_Run();

#define IMPL_RUN_TEST(func) \
extern "C" wfcStatus  XTest_##func##_Run()

#define IMPL_VERIFY_TEST(func) \
extern "C" wfcStatus  XTest_##func##_Verify()

#define IMPL_DEFAULT_RUN_TEST(func) \
IMPL_RUN_TEST(func) \
{ \
	wfcStatus ret = TestFwkHlp::ValidateInputBeforeRun(); \
	TK_ERROR_RETURN(ret) \
	return RUN_TEST(func); \
}

#define IMPL_DEFAULT_VERIFY_TEST(func) \
IMPL_VERIFY_TEST(func) \
{ \
	return TestFwk().VerifyAllOutputDirs(); \
}

#define TK_ERROR(eval) ((eval) != wfcTK_NO_ERROR)
#define TK_ERROR_RETURN(eval) \
{\
	wfcStatus ret1 = (eval); \
	if (ret1 != wfcTK_NO_ERROR) return ret1; \
}

#define FAILED_RETURN(eval, ret, msg)\
{ \
	if (!(eval)) \
	{ \
		XLOG_ERROR(msg); \
		return ret; \
	} \
}

#define FAILED_RETURN_VOID(eval, msg)\
{ \
	if (!(eval)) \
	{ \
		XLOG_ERROR(msg); \
		return; \
	} \
} 

#define RUN_TEST_CALLBACK(func) XTest_##func##_Run
#define VERIFY_TEST_CALLBACK(func) XTest_##func##_Verify

#define DEFINE_TESTCOMMAND(cmdName, desc) \
	{ \
		TestCommand cmd; \
		cmd.m_cmdName = _STRINGIZE(XTest_##cmdName); \
		cmd.m_cmdLabel= desc; \
		cmd.m_runCallback = RUN_TEST_CALLBACK(cmdName); \
		cmd.m_verifyCallback = VERIFY_TEST_CALLBACK(cmdName); \
		auto pSess = pfcGetProESession(); \
		pfcUICommand_ptr pCmd = pSess->UICreateCommand(cmd.m_cmdName.c_str(), new TestCommandListener(cmd.m_cmdName)); \
		if (cmd.m_cmdLabel.empty()) \
			pCmd->Designate(MSG_FILE, cmd.m_cmdName.c_str(), cmd.m_cmdName.c_str(), cmd.m_cmdName.c_str()); \
		else \
			pCmd->Designate(MSG_FILE, cmd.m_cmdName.c_str(), cmd.m_cmdLabel.c_str(), cmd.m_cmdName.c_str()); \
		auto pCmdBracket = new TestCommandBracketListener(cmd.m_cmdName); \
		pCmd->AddActionListener(pCmdBracket); \
		m_testCommands.push_back(cmd); \
		LOG_DEBUG_FMT(L"Test command created: %s", RMXUtils::s2ws(cmd.m_cmdName).c_str()); \
	}

#define DEFINE_HLP_COMMAND(cmdName) \
	{ \
		TestCommand cmd; \
		cmd.m_cmdName = _STRINGIZE(XTest_##cmdName); \
		cmd.m_cmdLabel= cmd.m_cmdName; \
		cmd.m_runCallback = RUN_TEST_CALLBACK(cmdName); \
		cmd.m_verifyCallback = nullptr; \
		auto pSess = pfcGetProESession(); \
		pfcUICommand_ptr pCmd = pSess->UICreateCommand(cmd.m_cmdName.c_str(), new TestCommandListener(cmd.m_cmdName)); \
		pCmd->Designate(MSG_FILE, cmd.m_cmdLabel.c_str(), cmd.m_cmdLabel.c_str(), cmd.m_cmdLabel.c_str()); \
		m_testCommands.push_back(cmd); \
		LOG_DEBUG_FMT(L"Test helper command created: %s", RMXUtils::s2ws(cmd.m_cmdName).c_str()); \
	}

namespace TestFwkHlp
{
	std::string ReadScripts(const std::wstring& fileName);
	bool UnzipFile(const wstring & zipFile, const wstring & destDir);

	bool OpenNotepad(const std::wstring& filePath);
	void RemoveFiles(const std::wstring& dir, bool removeDir);

	wfcStatus AnyProtectedInCurrentModel();
	wfcStatus AnyProtectedInSelection();
	wfcStatus IsCurrentModelProtected();
	wfcStatus ValidateInputBeforeRun();

	void GetDepNxlFiles(pfcModel_ptr pParentMdl, std::set<std::wstring>& nxlFiles);
	void GetDepNxlFiles(const std::wstring& parentFilePath, std::set<std::wstring>& nxlFiles);
	std::string GetNxlFileTags(const std::wstring & filePath);

	wstring GetNativeFilePath(const std::wstring & filePath);

	bool IsAsm(const std::wstring & filePath);
	bool IsDrw(const std::wstring & filePath);

	bool IsSaveFileIterationEnabled();

	wstring EnsureNxlExt(const std::wstring & filePath);

}; // namespace TestFwkHlp

