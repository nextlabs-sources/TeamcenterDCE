
#include "TestFwkHlp.h"
#include "TestFwk.h"

#include <PathEx.h>
#include "XTestLogger.h"
#include <RMXUtils.h>
#include <OtkXUtils.h>
#include <regex>
#include <pfcGlobal.h>

#include <LibRMX.h>
#include <set>

using namespace std;
using namespace OtkXUtils;

IMPL_DEFAULT_RUN_TEST(Save)
IMPL_VERIFY_TEST(Save)
{
	return TestFwk().VerifyCurrentModel();
}

IMPL_RUN_TEST(SaveAs)
{
	const wstring& testMdlPath = TestFwk().GetTestModel();
	if (TestFwkHlp::IsAsm(testMdlPath))
		return RUN_TEST(SaveAsAsm);
	else
		return RUN_TEST(SaveAs);
}

IMPL_VERIFY_TEST(SaveAs)
{
	wfcStatus ret = wfcTK_NO_ERROR;
	const wstring& testMdlPath = TestFwk().GetTestModel();
	OtkXFileNameData nameData(testMdlPath);
	wstring fileNameSuffix(L"." + nameData.GetExtension());
	if (TestFwkHlp::IsSaveFileIterationEnabled())
		fileNameSuffix += L".1";

	vector<wstring> newFiles;
	bool fileCreated = false;
	newFiles.push_back(TestFwk().GetOutputRPMDir() + L"\\saveas_rpm" + fileNameSuffix);
	newFiles.push_back(TestFwk().GetOutputNonRPMDir() + L"\\saveas_nonrpm" + fileNameSuffix);

	for (const auto& newFile : newFiles)
	{
		ret = TestFwk().VerifyFile(newFile);
		TK_ERROR_RETURN(ret);
	}

	return ret;
}

IMPL_DEFAULT_RUN_TEST(Backup)
IMPL_VERIFY_TEST(Backup)
{
	wfcStatus ret = wfcTK_NO_ERROR;
	
	const wstring& testMdlPath = TestFwk().GetTestModel();
	OtkXFileNameData nameData(testMdlPath);
	wstring fileNameSuffix(nameData.GetFileName());
	if (TestFwkHlp::IsSaveFileIterationEnabled())
		fileNameSuffix += L".1";

	vector<wstring> newFiles;
	newFiles.push_back(TestFwk().GetOutputRPMDir() + L"\\" + fileNameSuffix);
	newFiles.push_back(TestFwk().GetOutputNonRPMDir() + L"\\" + fileNameSuffix);
	
	for (const auto& newFile : newFiles)
	{
		ret = TestFwk().VerifyFile(newFile);
		TK_ERROR_RETURN(ret);
	}

	return ret;
}

IMPL_DEFAULT_RUN_TEST(IpemSave)
IMPL_VERIFY_TEST(IpemSave)
{
	return TestFwk().VerifyCurrentModel();
}

static std::set<std::wstring, ICaseKeyLess> s_origFilesInFolder;
std::set<std::wstring, ICaseKeyLess> _ListFilesInFolder(const std::wstring& dir)
{
	std::set<std::wstring, ICaseKeyLess> fList;
	WIN32_FIND_DATA findData;
	wstring searchDir(dir + L"\\*");
	HANDLE hFind = FindFirstFile(searchDir.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		CPathEx srcFile;
		do
		{
			if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				srcFile = dir.c_str();
				srcFile /= findData.cFileName;
				fList.insert(srcFile.ToString());
			}
		} while (FindNextFile(hFind, &findData));
		FindClose(hFind);
	}

	return fList;
}

wfcStatus _VerifyIpemFiles()
{
	wfcStatus ret = wfcTK_NO_ERROR;
	const wstring& testMdlPath = TestFwk().GetTestModel();
	OtkXFileNameData nameData(testMdlPath);
	std::set<std::wstring, ICaseKeyLess> fList = _ListFilesInFolder(nameData.GetDirectoryPath());
	std::set<std::wstring, ICaseKeyLess> fListToVerify;
	for (const auto& f : fList)
	{
		if (s_origFilesInFolder.find(f) == s_origFilesInFolder.end())
		{
			// This is new generated file, verify it
			if (OtkX_IsSupportedFileType(f.c_str()))
			{
				OtkXFileNameData fName(f);
				bool needVerify = true;
				if (fName.GetBaseName().size() > 36)
				{
					// Ignore the backup temp files which contain GUID prefix
					auto pos = fName.GetBaseName().find_first_of(L"_");
					auto prefix = fName.GetBaseName().substr(0, pos);
					static const std::wregex regex(L"[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}");
					std::match_results<std::wstring::const_iterator> match;
					if (std::regex_match(prefix, match, regex))
					//if (prefix.length() == 36)
					{
						needVerify = false;
					}
				}

				if (needVerify)
				{
					CPathEx tmpFile(TestFwk().GetOutputRPMDir().c_str());
					tmpFile /= fName.GetFileName().c_str();
					if (fName.GetVersion() > 0)
					{
						tmpFile += L".";
						tmpFile += std::to_wstring(fName.GetVersion()).c_str();
					}
					wstring nxlFPath(f.c_str());
					nxlFPath = TestFwkHlp::EnsureNxlExt(nxlFPath);
					if (CPathEx::IsFile(nxlFPath))
					{
						if (RMX_CopyNxlFile(f.c_str(), tmpFile.c_str(), false))
						{
							TestFwk().UpdateTestFileInfo(f, tmpFile.ToString());
							fListToVerify.insert(tmpFile.ToString());
						}
					}
					else if (CopyFile(f.c_str(), tmpFile.c_str(), false))
					{
						fListToVerify.insert(tmpFile.ToString());
					}
					
				}
			}
		}
	}
	
	for (const auto& f : fListToVerify)
	{
		ret = TestFwk().VerifyFile(f);
		TK_ERROR_RETURN(ret);
	}

	return ret;
}

IMPL_RUN_TEST(IpemSaveAs)
{
	wfcStatus ret = TestFwkHlp::ValidateInputBeforeRun();
	TK_ERROR_RETURN(ret);
	
	const wstring& testMdlPath = TestFwk().GetTestModel();
	OtkXFileNameData nameData(testMdlPath);
	s_origFilesInFolder = _ListFilesInFolder(nameData.GetDirectoryPath());
	return RUN_TEST(IpemSaveAs);
}

IMPL_VERIFY_TEST(IpemSaveAs)
{
	return _VerifyIpemFiles();
}

IMPL_RUN_TEST(IpemSaveAll)
{
	wfcStatus ret = TestFwkHlp::ValidateInputBeforeRun();
	TK_ERROR_RETURN(ret);

	const wstring& testMdlPath = TestFwk().GetTestModel();
	OtkXFileNameData nameData(testMdlPath);
	s_origFilesInFolder = _ListFilesInFolder(nameData.GetDirectoryPath());
	return RUN_TEST(IpemSaveAll);
}

IMPL_VERIFY_TEST(IpemSaveAll)
{
	return _VerifyIpemFiles();
}