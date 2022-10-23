#include "TestFwk.h"

#include <LibRMX.h>
#include <OtkXModel.h>
#include <OtkXTypes.h>
#include <OtkXUtils.h>
#include <PathEx.h>
#include "XTestLogger.h"
#include <RMXUtils.h>
#include "XTestLogger.h"
#include <SysErrorMessage.h>

#include <RMXTagHlp.h>

#include <pfcGlobal.h>
#include <wfcSession.h>

#include "TestFwkHlp.h"
#include "TestCaseInitializer.h"

using namespace OtkXUtils;

DECLAR_TESTFUNCS()

typedef std::vector<std::string> TTagValueList;
// Case insensitive map
typedef std::map<std::string, TTagValueList, TagKeyComp> TTagList;

string _TagListToString(TTagList& list)
{
	string ret = "{";
	for (const auto& tag : list)
	{
		if (tag.first.empty() || tag.second.size() == 0)
			continue;
		ret += "\"" + tag.first + "\":[";
		for (const auto& value : tag.second)
		{
			ret += +"\"" + value + "\",";
		}
		ret.erase(ret.length() - 1);
		ret += "],";
	}
	ret.erase(ret.length() - 1);
	ret += "}";

	return ret;
}

void _ExtratTags(const string& tags, TTagList& tagList, string& originFile)
{
	size_t nextPos = 0;
	while (nextPos < tags.length())
	{
		size_t startPosKey = tags.find_first_of('\"', nextPos);
		if (startPosKey == string::npos)
			break;

		size_t endPosKey = tags.find_first_of('\"', startPosKey + 1);
		if (endPosKey == string::npos)
			break;

		const string& key = tags.substr(startPosKey + 1, endPosKey - startPosKey - 1);

		size_t openBracket = tags.find_first_of('[', endPosKey + 1);
		if (openBracket == string::npos)
			break;
		size_t closeBracket = tags.find_first_of(']', openBracket + 1);
		if (closeBracket == string::npos)
			break;

		// In case the inner bracket like "gov_classificcation" : ["[ILLEGAL ARGS]"]
		size_t nextOpenBracket = tags.find_first_of('[', openBracket + 1);
		while (true)
		{
			if (closeBracket < nextOpenBracket)
			{
				break;
			}
			else
			{
				closeBracket = tags.find_first_of(']', closeBracket + 1);
				nextOpenBracket = tags.find_first_of('[', nextOpenBracket + 1);
			}
		}
		size_t startPosVal = tags.find_first_of('\"', openBracket + 1);
		if (startPosVal == string::npos)
			break;

		while (startPosVal < closeBracket)
		{
			size_t endPosVal = tags.find_first_of('\"', startPosVal + 1);
			if (endPosVal == string::npos)
				break;

			const string& value = tags.substr(startPosVal + 1, endPosVal - startPosVal - 1);
			if (key == "NXL_ORIGIN")
				originFile = value;
			else
				tagList[key].push_back(value);

			startPosVal = tags.find_first_of('\"', endPosVal + 1);
		}

		nextPos = closeBracket;
	}

	for (auto & tag : tagList)
	{
		std::sort(tag.second.begin(), tag.second.end(), TagKeyComp());
	}
}

bool _CompareTags(const std::string & tags1, const std::string & tags2)
{
	TTagList tagList1;
	RMXLib::CTagCollection tagColl(tags1);
	tagList1 = tagColl.toList();
	for (auto & tag : tagList1)
	{
		std::sort(tag.second.begin(), tag.second.end(), TagKeyComp());
	}

	TTagList tagList2;
	RMXLib::CTagCollection tagColl2(tags2);
	tagList2 = tagColl.toList();
	for (auto & tag : tagList2)
	{
		std::sort(tag.second.begin(), tag.second.end(), TagKeyComp());
	}

	if (tagList1.size() == tagList2.size() &&
		std::equal(tagList1.begin(), tagList1.end(), tagList2.begin()))
	{
		return true;
	}

	return false;
}

void TestCommandListener::OnCommand()
{
	TestFwk().OnTestCommand(m_cmdName);
}

void TestCommandBracketListener::OnBeforeCommand()
{
	TestFwk().OnBeforeTestCommand(m_cmdName);
}

void TestCommandBracketListener::OnAfterCommand()
{
	TestFwk().OnAfterTestCommand(m_cmdName);
}

CTestFwk::CTestFwk()
{
	m_testOutputRoot = RMXUtils::getEnv(L"userprofile") + L"\\Documents\\CreoRMXTest_Output";
}


CTestFwk::~CTestFwk()
{
}

bool CTestFwk::Init()
{
	if (!InitWorkFolder())
		return false;
	LOG_INFO(L"Fwk information: ");
	LOG_INFO(L"\t-Install dir: " << m_fwkInstallDir.c_str());
	LOG_INFO(L"\t-Scripts dir: " << m_fwkScriptsDir.c_str());
	LOG_INFO(L"\t-Output dir: " << m_testOutputRoot.c_str());
	LOG_INFO(L"\t-Creo work dir: " << m_srcWorkDir.c_str());
	return SetupTestCases();
}

bool CTestFwk::InitWorkFolder()
{
	GetInstallDir();
	FAILED_RETURN(!m_fwkInstallDir.empty(), false, L"XTest folder not found in $CREO_RMX_ROOT");
#if defined(TESTFWK_INSTALL_ROOT)
	m_fwkScriptsDir = _W(_STRINGIZE(TESTFWK_INSTALL_ROOT));
	m_fwkScriptsDir.erase(0, 1); // erase the first quote
								 // Remove ".\"
	m_fwkScriptsDir.erase(m_fwkScriptsDir.length() - 2);
#else
	m_fwkScriptsDir = m_fwkInstallDir + L"scripts";
#endif

	/*wstring currTime;
	std::time_t tNow = std::time(nullptr);
	wchar_t str[20];
	if (std::wcsftime(str, sizeof(str), L"%Y%m%d%H%M%S", std::localtime(&tNow))) {
	currTime.assign(str);
	}
	*/
	
	m_testOutputRoot_RPM = m_testOutputRoot + L"\\RPM";
	m_testOutputRoot_NonRPM = m_testOutputRoot + L"\\Non RPM";
	if (CPathEx::IsDir(m_testOutputRoot_RPM))
	{
		TestFwkHlp::RemoveFiles(m_testOutputRoot_RPM, false);
	}
	else if (CPathEx::CreateDirectories(m_testOutputRoot_RPM))
	{
		XLOG_ERROR_FMT(L"Cannot create directory: %s", m_testOutputRoot_RPM.c_str());
		return false;
	}
	if (!RMX_AddRPMDir(m_testOutputRoot_RPM.c_str()))
		return false;

	if (CPathEx::IsDir(m_testOutputRoot_NonRPM))
	{
		TestFwkHlp::RemoveFiles(m_testOutputRoot_NonRPM, false);
		if (!RMX_RemoveRPMDir(m_testOutputRoot_NonRPM.c_str()))
			return false;
	}
	else if (CPathEx::CreateDirectories(m_testOutputRoot_NonRPM))
	{
		XLOG_ERROR_FMT(L"Cannot create directory: %s", m_testOutputRoot_NonRPM.c_str());
		return false;
	}

	m_srcWorkDir = OtkX_GetCurrentDirectory();
	
	return true;
}

bool CTestFwk::SetupTestCases()
{
	try
	{
		DEFINE_TESTCOMMANDS()
		return true;
	}
	OTKX_EXCEPTION_HANDLER();
	return false;
}

void CTestFwk::ResetTestCaseEnv()
{
	TestFwkHlp::RemoveFiles(m_testOutputRoot_NonRPM, false);
	TestFwkHlp::RemoveFiles(m_testOutputRoot_RPM, false);
	
	m_currTestCase.clear();
	m_failedTestCases.clear();
	m_testFileInfo.clear();
	m_passedFiles.clear();
	m_srcAllMdlToTest.clear();
	ReopenTestModel();

	// Take snapshot of current model and dependencies
	auto pCurrMdl = OtkXUtils::OtkX_GetCurrentModel();
	if (pCurrMdl != nullptr)
	{
		auto pXMdl = std::make_shared<COtkXCacheModel>(pCurrMdl);
		m_srcMdlToTest = *pXMdl;
		wstring origin = m_srcMdlToTest.GetOrigin();
		if (m_srcMdlToTest.IsProtected())
		{
			origin = TestFwkHlp::EnsureNxlExt(origin);
		}
		m_srcAllMdlToTest[origin] = pXMdl;
		COtkXModel xMdl(pCurrMdl);
		xMdl.TraverseDependencies([&](COtkXModel& xDepMdl) {
			wstring filePath(xDepMdl.GetOrigin());
			if (xDepMdl.IsProtected())
			{
				filePath = TestFwkHlp::EnsureNxlExt(filePath);
			}
			m_srcAllMdlToTest[filePath] = std::make_shared<COtkXCacheModel>(xDepMdl.GetModel());
			return false;
		});
	}

	RMX_RemoveRPMDir(m_testOutputRoot_NonRPM.c_str());
	ResetWorkDir();
}

wfcStatus CTestFwk::RunTestCase(const std::string & scriptFile)
{
	wstring scriptFName = RMXUtils::s2ws(scriptFile).c_str();
	LOG_DEBUG(L"Execute test scripts: " << scriptFName.c_str());
	try
	{
		pfcSession_ptr pSess = pfcGetProESession();
		string macros = TestFwkHlp::ReadScripts(m_fwkScriptsDir + L"\\" + scriptFName);
		pSess->RunMacro(CA2XS(macros.c_str()));
	}
	OTKX_EXCEPTION_HANDLER();
	return (wfcTK_NO_ERROR);
}

void CTestFwk::CompleteTestCase()
{
	if (!m_currTestCase.empty())
	{
		const auto& cmd = FindTestCommand(m_currTestCase);
		if (cmd.m_verifyCallback)
		{
			LOG_INFO(L">> Verify test case: " << RMXUtils::s2ws(cmd.m_cmdName));
			wfcStatus ret = cmd.m_verifyCallback();
			if (ret != wfcTK_NO_ERROR)
			{
				m_failedTestCases.push_back(m_currTestCase);
			}
		}

		// Final verification to ensure all expected types are generated and tested
		auto fileTypes = cmd.m_cmdLabel;
		if (!fileTypes.empty())
		{
			stringstream tokValues(fileTypes);
			string val;
			bool missingFileType = false;
			// Tokenizing by '|'
			while (std::getline(tokValues, val, '|'))
			{
				wstring fType = RMXUtils::s2ws(val);
				bool allTypePassed = false;
				for (const auto& file : m_passedFiles)
				{
					if (wcsstr(file.first.c_str(), fType.c_str()) != NULL)
					{
						allTypePassed = true;
						break;
					}
				}
				
				if (!allTypePassed)
				{
					XLOG_ERROR_FMT(L"Expected output file not found for this type: %s", fType.c_str());
					missingFileType = true;
				}
			}
			if (missingFileType)
				m_failedTestCases.push_back(m_currTestCase);
		}
		auto pSess = pfcGetProESession();
		if (pSess)
		{
			xstring dir = CA2XS(RMXUtils::ws2s(m_srcWorkDir).c_str());
			pSess->ChangeDirectory(dir);
		}

		LOG_INFO(L">> Finish test case: " << RMXUtils::s2ws(m_currTestCase).c_str());
		
		ReportTestResult();
		m_currTestCase.clear();
		m_testFileInfo.clear();
		ReopenTestModel();
		LOG_INFO(L"------------------------------------------------ >>");

	}

	
}

void CTestFwk::ReportTestResult()
{
	if (m_failedTestCases.size() > 0)
	{
		XLOG_ERROR_FMT(L">> Total failure: %d. Please investigate!!", m_failedTestCases.size());
		XLOG_ERROR(L">> Failed test cases:");
		for (const auto& func : m_failedTestCases)
		{
			XLOG_ERROR(L"\t-" << RMXUtils::s2ws(func).c_str());
		}
		wstring allTestedFiles(L">> All passed files:");
		for (const auto& file : m_passedFiles)
		{
			allTestedFiles = allTestedFiles + L"\n\t-" + file.first + L"\n\t\t-Source file: " + file.second.first + L"\n\t\t-Tags" +
				RMXUtils::s2ws(file.second.second);
		}
		LOG_INFO(allTestedFiles);
		const wstring& logDir = GetTestLogError();
		TestFwkHlp::OpenNotepad(logDir);
		m_failedTestCases.clear();
	}
	else
	{
		LOG_INFO(L">> All test cases passed.");
		// Generate detailed report for tested file list
		wstring allTestedFiles(L">> All passed files:");
		for (const auto& file : m_passedFiles)
		{
			allTestedFiles = allTestedFiles + L"\n\t-" + file.first + L"\n\t\t-Source file: " + file.second.first + L"\n\t\t-Tags" +
				RMXUtils::s2ws(file.second.second);
		}
		LOG_INFO(allTestedFiles);
		xstring msg;
		const TestCommand& cmd = FindTestCommand(m_currTestCase);
		if (!cmd.m_cmdLabel.empty())
		{
			LOG_INFO(L">> All tested types: " << cmd.m_cmdLabel.c_str());
			msg = xstring::Printf("[XTest]Test case passed:  '%s'\nFile types: '%s", m_currTestCase.c_str(), cmd.m_cmdLabel.c_str());
		}
		else
		{
			msg = xstring::Printf("[XTest]Test case passed:  '%s'\n", m_currTestCase.c_str());
		}
	/*	auto pSess = pfcGetProESession();
		xstringsequence_ptr msgs = xstringsequence::create();
		xstring msgLine(m_currTestCase.c_str());
		msgs->append(msgLine);
		pSess->UIDisplayMessage("msg_user.txt", "RMXTestPassed", msgs);*/

		
		OtkX_ShowMessageDialog(msg, pfcMESSAGE_INFO);
	}
}

void CTestFwk::ReopenTestModel()
{
	if (m_srcMdlToTest.IsValid())
	{
		string macros = "~Command `ProCmdModelErase`;\
			~Activate `0_std_confirm` `OK`;\
			!%CPCurrent object will be erased.Select associated objects to also erase.;\
			~Activate `file_erase` `sel_list`;\
			~Activate `file_erase` `OK`;\
			~ Command `ProCmdRecentFileOpen` `";
		macros += RMXUtils::ws2s(m_srcMdlToTest.GetOrigin().c_str());
		std::replace(macros.begin(), macros.end(), '\\', '/');
		macros += "`;";
		try
		{
			pfcSession_ptr pSess = pfcGetProESession();
			pSess->RunMacro(CA2XS(macros.c_str()));
		}
		OTKX_EXCEPTION_HANDLER();
	}

	/*_CloseAllModels();
	if (!m_cacheCurrentModelPath.empty())
	{
	OtkX_OpenFile(RMXUtils::ws2s(m_cacheCurrentModelPath).c_str());
	}*/
}

void CTestFwk::OnTestCommand(std::string& cmdName)
{
	const TestCommand& cmd = FindTestCommand(cmdName);
	if (cmd.m_runCallback)
	{
		wfcStatus ret = cmd.m_runCallback();
		if (ret != wfcTK_NO_ERROR)
		{
			m_failedTestCases.push_back(cmdName);
		}
	}
}

void CTestFwk::OnBeforeTestCommand(std::string& cmdName)
{
	ResetTestCaseEnv();
	m_currTestCase = cmdName;

	const TestCommand& cmd = FindTestCommand(cmdName);
	LOG_INFO(L"<< ------------------------------------------------");
	LOG_INFO(L"<< Run test case: ");
	LOG_INFO(L"\t - Test name: " << RMXUtils::s2ws(cmdName).c_str());	
	LOG_INFO(L"\t - File types: " << cmd.m_cmdLabel.c_str());
	LOG_INFO(L"\t - Model: " << m_srcMdlToTest.GetOrigin().c_str());
}

void CTestFwk::OnAfterTestCommand(std::string& cmdName)
{

}

wfcStatus CTestFwk::ViewInfo()
{
	pfcModel_ptr pMdl = GetSelectedModel();
	if (pMdl == nullptr)
		pMdl = OtkX_GetCurrentModel();

	if (pMdl == nullptr)
	{
		xstring msg("Current model or selection not found");
		OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
	}
	else
	{
		COtkXModel xMdl(pMdl);
		if (!xMdl.IsProtected())
		{
			xstring msg("Not protected file");
			OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
		}
		else
		{
			unsigned long fileRights = 0;
			RMX_GetRights(xMdl.GetOrigin().c_str(), fileRights);
			xstring msg = xstring::Printf("Protected model: %s\nPermission: ", (const char*)pMdl->GetInstanceName());;
			if (HAS_BIT(fileRights, RMX_RIGHT_VIEW))
			{
				msg += "View";
			}
			if (HAS_BIT(fileRights, RMX_RIGHT_EDIT))
			{
				msg += ", Edit";
			}
			if (HAS_BIT(fileRights, RMX_RIGHT_SAVEAS) || HAS_BIT(fileRights, RMX_RIGHT_DOWNLOAD))
			{
				msg += ", Save As";
			}
			if (HAS_BIT(fileRights, RMX_RIGHT_PRINT))
			{
				msg += ", Print";
			}
			if (HAS_BIT(fileRights, RMX_RIGHT_DECRYPT))
			{
				msg += ", Extract";
			}
			msg += "\n";
			string tags = xMdl.GetTags();
			if (!tags.empty())
			{
				msg += xstring::Printf("Tags: %s", tags.c_str());
			}

			OtkX_ShowMessageDialog(msg, pfcMESSAGE_INFO);
		}
	}
	return wfcTK_NO_ERROR;
}

wfcStatus CTestFwk::ViewModelInfo()
{
	auto pMdl = GetSelectedModel();
	if (pMdl == nullptr)
		pMdl = OtkX_GetCurrentModel();
	if (pMdl == nullptr)
	{
		xstring msg("Current model or selection not found");
		OtkX_ShowMessageDialog(msg, pfcMESSAGE_ERROR);
	}
	else
	{
		xstring info("Model:"); 
		OTKX_MODEL_TOSTR(pMdl, info);
		info += "\n\nModel Descriptor:";
		OTKX_MODELDESCR_TOSTR(pMdl->GetDescr(), info);
	
		info += "\n\nDependent files:";
		COtkXModel xMdl(pMdl);
		xMdl.TraverseDependencies([&](COtkXModel& xDepMdl) {
			info = info + L"\n\t - " + xDepMdl.GetModel()->GetOrigin();
			return false;
		});
		OtkX_ShowMessageDialog(info, pfcMESSAGE_INFO);
	}
	
	return wfcTK_NO_ERROR;
}

bool CTestFwk::IsTestCaseRunning()
{
	bool running = !m_currTestCase.empty();
	LOG_INFO_FMT(L"IsTestCaseRunning %s", running ? L"true" : L"false");
	return running;
}

wstring CTestFwk::GetInstallDir()
{
	if (m_fwkInstallDir.empty())
	{
		try
		{
			auto pSess = wfcWSession::cast(pfcGetProESession());
			m_fwkInstallDir = CXS2W(pSess->GetApplicationTextPath());
			if (m_fwkInstallDir.back() != '\\')
				m_fwkInstallDir += '\\';
			
			// FindFirstFile does not support short format of path
			wchar_t bufferDir[1024];
			if (GetLongPathName(m_fwkInstallDir.c_str(), &bufferDir[0], _MAX_DIR) == 0)
				return wstring(L"");

			if (!CPathEx::IsDir(m_fwkInstallDir))
				m_fwkInstallDir.clear();
		}
		OTKX_EXCEPTION_HANDLER();
	}

	return m_fwkInstallDir;
}

void CTestFwk::AddTestFileInfo(const std::wstring& testNxlFile, const std::wstring& nxlOrigin, const std::string& tags)
{
	wstring originFile = TestFwkHlp::EnsureNxlExt(nxlOrigin);
	// If Drw/asm is not nxl file, but have some nxl dependencies, it can be nxlorigin
	if (!CPathEx::IsFile(originFile))
		originFile = nxlOrigin; 
	m_testFileInfo[TestFwkHlp::EnsureNxlExt(testNxlFile)] = std::make_pair(originFile, tags);
}

std::pair<wstring, string> CTestFwk::FindNxlOrigin(const std::wstring & testNxlFile) const
{
	auto elem = m_testFileInfo.find(testNxlFile);
	if (elem != m_testFileInfo.end())
		return elem->second;

	OtkXFileNameData nameData(testNxlFile);
	for (const auto& f : m_testFileInfo)
	{
		if (wcsstr(f.first.c_str(), nameData.GetFileName().c_str()) != NULL)
			return f.second;
	}
	return std::make_pair(L"", "");
}

TCacheModelPtr CTestFwk::FindCacheModel(const std::wstring & filePath)
{
	auto elem = m_srcAllMdlToTest.find(filePath);
	if (elem != m_srcAllMdlToTest.end())
		return elem->second;

	return nullptr;
}

void CTestFwk::UpdateTestFileInfo(const std::wstring & origTestFile, const std::wstring & newTestFile)
{
	wstring oldFile = TestFwkHlp::EnsureNxlExt(origTestFile);
	auto& elem = m_testFileInfo.find(oldFile);
	if (elem != m_testFileInfo.end())
	{
		wstring newFile = TestFwkHlp::EnsureNxlExt(newTestFile);
		m_testFileInfo[newFile] = elem->second;
		m_testFileInfo.erase(elem);
	}
		
}

wfcStatus  CTestFwk::VerifyCurrentModel()
{
	LOG_INFO(L"Verifying current model...");
	if (!m_srcMdlToTest.IsProtected())
	{
		LOG_INFO(L"Ignore verifying. Current model not protected");
		return wfcTK_NO_ERROR;
	}
		 
	auto pMdl = OtkX_GetCurrentModel();
	FAILED_RETURN(pMdl != nullptr, wfcTK_BAD_INPUTS, L"Current model not found");
	COtkXModel xMdl(pMdl);
	wstring newFilePath = xMdl.GetOrigin();
	return VerifySingleFile(newFilePath);
}

wfcStatus  CTestFwk::VerifySelection()
{
	try
	{
		pfcSession_ptr pSess = pfcGetProESession();
		pfcSelectionBuffer_ptr pSelBuffer = pSess->GetCurrentSelectionBuffer();
		pfcSelections_ptr pSels = pSelBuffer->GetContents();

		FAILED_RETURN(pSels != nullptr, wfcTK_BAD_INPUTS, L"Selection not found");
		xint numSels = pSels->getarraysize();
		FAILED_RETURN(numSels > 0, wfcTK_BAD_INPUTS, L"Selection not found");

		for (int i = 0; i < numSels; ++i)
		{
			pfcModel_ptr pSelMdl = pSels->get(i)->GetSelModel();
			if (pSelMdl != nullptr)
			{
				xstring newFilePath = pSelMdl->GetOrigin();
				if (!RMX_IsProtectedFile(CXS2W(newFilePath)))
				{
					return wfcTK_GENERAL_ERROR;
				}
			}
		}
	}
	OTKX_EXCEPTION_HANDLER();

	return wfcTK_NO_ERROR;
}

wfcStatus  CTestFwk::VerifyNxlFileTags(const std::wstring& filePath)
{
	LOG_INFO_FMT(L"Verify tags for '%s'...", filePath.c_str());
	
	auto nxlOrigin = FindNxlOrigin(filePath);
	if (nxlOrigin.first.empty() || nxlOrigin.second.empty())
	{
		XLOG_ERROR(L"NXL origin not existed: " << filePath.c_str());
		return wfcTK_BAD_INPUTS;
	}

	string allOrigTags;
	wstring origFilePath(nxlOrigin.first);
	if (wcsstr(origFilePath.c_str(), L".prt") == NULL ||
		wcsstr(origFilePath.c_str(), L".drw") == NULL ||
		wcsstr(origFilePath.c_str(), L".asm") == NULL ||
		wcsstr(filePath.c_str(), L".asm") != NULL || wcsstr(filePath.c_str(), L".drw") != NULL || wcsstr(filePath.c_str(), L".prt") != NULL)
	{
		allOrigTags = TestFwkHlp::GetNxlFileTags(nxlOrigin.first);
	}
	else
	{
		auto pXMdl = FindCacheModel(origFilePath);
		if (pXMdl == nullptr)
		{

			XLOG_ERROR(L"Cache model not found: " << nxlOrigin.first.c_str());
			return wfcTK_BAD_INPUTS;
		}

		// Export to diff format, merge tags from all dependencies
		std::vector<std::pair<std::wstring, std::string>> vecOrigTags;
		pXMdl->GetTagsWithDep(vecOrigTags);
		for (const auto& elem : vecOrigTags)
		{
			allOrigTags += elem.second;
		}
		char* origTags2 = nullptr;
		RMX_MergeTags(allOrigTags.c_str(), &origTags2);
		if (origTags2 != nullptr)
		{
			allOrigTags = origTags2;
			RMX_ReleaseArray((void*)origTags2);
		}
	}
	/*if (!CPathEx::IsFile(wOrigFilePath))
	{	
		if (wcsstr(filePath.c_str(), L"rpm_zip") != NULL)
		{
			static const wchar_t* imageExts[] = { L".jpg", L".bmp", L".tif", L".gif", L".cgm" };
			for (const auto& ext : imageExts)
			{
				if (wcsstr(filePath.c_str(), ext) != NULL)
				{
					CPathEx destPath(filePath.c_str());
					CPathEx origPath(wOrigFilePath.c_str());
					CPathEx origPath2 = destPath.GetParentPath().c_str();
					origPath2 /= origPath.GetFileName().c_str();
					origPath2 += origPath.GetExtension().c_str();
					wOrigFilePath = origPath2.ToString();

					if (!CPathEx::IsFile(wOrigFilePath))
					{
						XLOG_ERROR(L"NXL origin file not exist: " << filePath.c_str());
						return wfcTK_BAD_INPUTS;
					}

					break;
				}
			}
		}*/
		
		/*XLOG_ERROR(L"NXL origin file not exist: " << filePath.c_str());
		return wfcTK_BAD_INPUTS;*/
	//}

	auto tags = TestFwkHlp::GetNxlFileTags(filePath);
	TTagList tagList;

	wfcStatus ret = wfcTK_GENERAL_ERROR;

	if (_CompareTags(tags, allOrigTags))
	{
		LOG_INFO_FMT(L"Comparing tags: Same.\n\t-source: %s\n\t-sourcetags%s \n\t-target: %s\n\t-tagettags%s", nxlOrigin.first.c_str(), RMXUtils::s2ws(allOrigTags).c_str(),
			filePath.c_str(), RMXUtils::s2ws(tags).c_str());
		ret =  wfcTK_NO_ERROR;
	}
	else
	{
		XLOG_ERROR_FMT(L"Comparing tags: Diff!! \n\t-source: %s\n\t-sourcetags%s \n\t-target: %s\n\t-targettags%s", nxlOrigin.first.c_str(), RMXUtils::s2ws(allOrigTags).c_str(),
			filePath.c_str(), RMXUtils::s2ws(tags).c_str());
	}		

	// Record all verified files which passed test case. 
	// Will have last verification to ensure all output file types are verified and passed for export
	if (ret == wfcTK_NO_ERROR)
		m_passedFiles[filePath] = std::make_pair(nxlOrigin.first, tags);
	return ret;
}

wfcStatus CTestFwk::VerifySingleFile(const std::wstring & filePath)
{
	wstring fileToVerify = TestFwkHlp::EnsureNxlExt(filePath);
	
	if (!CPathEx::IsFile(fileToVerify))
	{
		XLOG_ERROR_FMT(L">> Verify file existing: No. %s", filePath.c_str());
		return (wfcTK_E_NOT_FOUND);
	}
	if (!RMX_IsProtectedFile(fileToVerify.c_str()))
	{
		XLOG_ERROR_FMT(L">> Verify protected: Miss. %s", filePath.c_str());
		return (wfcTK_GENERAL_ERROR);
	}
	else
	{
		LOG_INFO_FMT(L">> Verify protected: Correct. %s", filePath.c_str());
		return VerifyNxlFileTags(fileToVerify);
	}
}

void CTestFwk::ResetWorkDir()
{
	auto pSess = pfcGetProESession();
	if (pSess)
	{
		xstring dir = CA2XS(RMXUtils::ws2s(m_srcWorkDir).c_str());
		pSess->ChangeDirectory(dir);
	}
}

pfcModel_ptr CTestFwk::GetSelectedModel()
{

	pfcModel_ptr pMdl = nullptr;
	try
	{
		pfcSession_ptr pSess = pfcGetProESession();
		pfcSelectionBuffer_ptr pSelBuffer = pSess->GetCurrentSelectionBuffer();
		pfcSelections_ptr pSels = pSelBuffer->GetContents();

		if (pSels != nullptr)
		{
			xint numSels = pSels->getarraysize();
			if (numSels > 0)
			{
				pMdl = pSels->get(0)->GetSelModel();
			}
		}
	}
	OTKX_EXCEPTION_HANDLER();
	return pMdl;
}

wfcStatus  CTestFwk::VerifyParentFile(const std::wstring& asmFilePath)
{	
	// Verify the target parent file in case the root model is protected.
	wstring fileToVerify(asmFilePath);
	if (m_srcMdlToTest.IsProtected())
	{
		LOG_INFO(L"Verify parent file: " << fileToVerify.c_str());
		fileToVerify = TestFwkHlp::EnsureNxlExt(fileToVerify);
		TK_ERROR_RETURN(VerifySingleFile(fileToVerify));
	}

	CPathEx pathEx(fileToVerify.c_str());
	WIN32_FIND_DATA findData;
	wstring searchDir(pathEx.GetParentPath() + L"\\*");
	HANDLE hFind = FindFirstFile(searchDir.c_str(), &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		XLOG_ERROR_FMT(L"Cannot find dir '%s'", asmFilePath.c_str());
		return wfcTK_GENERAL_ERROR;
	}

	int nxlFileCount = 0;
	CPathEx srcFile;
	do
	{
		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			srcFile = pathEx.GetParentPath().c_str();
			srcFile /= findData.cFileName;

			if (RMX_IsProtectedFile(srcFile.c_str()))
			{
				wstring fileToVerify = TestFwkHlp::EnsureNxlExt(srcFile.ToString());
				if (m_testFileInfo.find(fileToVerify) == m_testFileInfo.end())
				{
					XLOG_ERROR_FMT(L"Unexpected nxl file found '%s'", srcFile.c_str());
					return wfcTK_GENERAL_ERROR;
				}

				nxlFileCount++;

				LOG_INFO_FMT(L">> Verify protected: Correct. %s", srcFile.c_str());
				VerifyNxlFileTags(fileToVerify);
			}
		}
	} while (FindNextFile(hFind, &findData));
	FindClose(hFind);

	return wfcTK_NO_ERROR;

}

wfcStatus CTestFwk::VerifyDepNxlFiles(pfcModel_ptr pParentMdl, set<wstring>& nxlFiles)
{
	set<wstring> allNxlFiles;
	TestFwkHlp::GetDepNxlFiles(pParentMdl, allNxlFiles);
	for (const auto& nxlFile : allNxlFiles)
	{
		TK_ERROR_RETURN(VerifyNxlFileTags(nxlFile));
		nxlFiles.insert(nxlFile);
	}

	return wfcTK_NO_ERROR;
}

wfcStatus CTestFwk::VerifyFile(const wstring& filePath)
{
	if (TestFwkHlp::IsAsm(filePath) || TestFwkHlp::IsDrw(filePath))
	{
		return VerifyParentFile(filePath);
	}
	else
	{
		return VerifySingleFile(filePath);
	}
	
}

wfcStatus CTestFwk::VerifyAllOutputDirs()
{
	if (!VerifyFolder(m_testOutputRoot_RPM) ||
		!VerifyFolder(m_testOutputRoot_NonRPM))
	{
		return (wfcTK_GENERAL_ERROR);
	}

	return (wfcTK_NO_ERROR);
}

bool CTestFwk::VerifyFolder(const std::wstring & dir)
{
	if (dir.empty())
	{
		LOG_ERROR(L"VerifyFolder: No dir specified");
		return false;
	}

	LOG_INFO_FMT(L"Verifying all files in folder '%s'...", dir.c_str());
	WIN32_FIND_DATA findData;
	wstring searchDir(dir + L"\\*");
	HANDLE hFind = FindFirstFile(searchDir.c_str(), &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		XLOG_ERROR_FMT(L"Cannot find dir '%s'", dir.c_str());
		if (GetLastError() != 0)
			XLOG_ERROR_FMT(L"error: '%s'", (LPCTSTR)CSysErrorMessage(GetLastError()));
		return false;
	}

	CPathEx srcFile;
	do
	{
		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			srcFile = dir.c_str();
			srcFile /= findData.cFileName;
			if (wcsstr(findData.cFileName, L".log") != NULL ||
				wcsicmp(findData.cFileName, L"manifest.xml") == 0)
				continue;

			if (wcsicmp(srcFile.GetExtension().c_str(), L".zip") == 0 ||
				wcsicmp(srcFile.GetExtension().c_str(), L".pvz") == 0 ||
				wcsicmp(srcFile.GetExtension().c_str(), L".edz") == 0)
			{
				//Unzip and verify the folder
				const wstring unzipDir = srcFile.GetParentPath() + L"\\" + srcFile.GetFileName();
				TestFwkHlp::RemoveFiles(unzipDir, false);

				if (!TestFwkHlp::UnzipFile(srcFile.ToString(), unzipDir))
					return false;

				Sleep(2000);
				if (CPathEx::IsDir(unzipDir))
				{
					LOG_INFO_FMT(L"Unzip file : %s", srcFile.c_str());
					if (!VerifyFolder(unzipDir))
						return false;
				}
				else
				{
					XLOG_ERROR_FMT(L"Failed to unzip file: %s", srcFile.c_str());
				}
			}
			else
			{
				wstring fileToVerify = TestFwkHlp::EnsureNxlExt(srcFile.ToString());

				if (CPathEx::IsFile(fileToVerify))
				{
					TK_ERROR_RETURN(VerifySingleFile(srcFile.ToString()));
				}
			}
				
			/*else if (!RMX_IsProtectedFile(srcFile.c_str()))
			{
				XLOG_ERROR_FMT(L">> Verify protected: Miss. %s", srcFile.c_str());
				return (wfcTK_GENERAL_ERROR);
			}
			else
			{
				LOG_INFO_FMT(L">> Verify protected: Correct. %s", srcFile.c_str());
				TK_ERROR_RETURN(VerifyNxlFileTags(srcFile.ToString()));
			}*/
		}
	} while (FindNextFile(hFind, &findData));
	FindClose(hFind);

	return true;
}

XTESTAPI bool IsTestCaseRunning()
{
	return CTestFwk::GetInstance().IsTestCaseRunning();
}

XTESTAPI void NotifyTestFile(const wchar_t* testNxlFile, const wchar_t* nxlOrigin, const char* tags)
{
	if (CTestFwk::GetInstance().IsTestCaseRunning())
	{
		wstring strTestFile(testNxlFile);
		wstring strOrigin(nxlOrigin);
		CTestFwk::GetInstance().AddTestFileInfo(strTestFile, strOrigin, string(tags));
	}	
}

XTESTAPI void DumpCachedNxlModels(const std::map<std::wstring, std::wstring>& mdls)
{
	wstring msg(L"Cached models in RMX session:");
	for (const auto& m : mdls)
	{
		msg = msg + L"\n\t-" + m.first + L" => " + m.second;
	}
	LOG_INFO(msg.c_str());
}