#include "uTestInc.h"

#include <string>
#include <windows.h>
#include <RMXUtils.h>
#include <SysErrorMessage.h>
#include <PathEx.h>
#include <RMXTagHlp.h>

using namespace std;

namespace RMXUTest
{
	TEST_CLASS(CUtilsUTest)
	{
	public:
		TEST_CLASS_INITIALIZE(CUtilsUTest_Class_Init)
		{
			UTEST_LOG("In UtilsUTest Class Initialize");
		}

		TEST_METHOD_INITIALIZE(CUtilsUTest_Method_Init)
		{
			UTEST_LOG("In UtilsUTest Method Initialize");
		}

		TEST_METHOD(TestSysErrorMessage)
		{
			UTEST_LOG(L"TestSysErrorMessage");
			CSysErrorMessage err(ERROR_ACCESS_DENIED);
			Assert::IsTrue(err.GetCode() == ERROR_ACCESS_DENIED, L"Invalid error code");
			Assert::IsTrue(err.GetMsg() != nullptr, L"Invalid error message");
			Logger::WriteMessage((LPCTSTR)err);

			SetLastError(ERROR_FILE_NOT_FOUND);
			Assert::IsFalse(CSysErrorMessage(GetLastError()), L"Failure for bool() operator");
			Assert::IsTrue(wcslen((LPCTSTR)CSysErrorMessage(GetLastError())) > 0, L"Failure for LPCTSTR() operator");
			SetLastError(ERROR_SUCCESS);
		}

		TEST_METHOD(TestCPathEX)
		{
			UTEST_LOG(L"TestCPathEX");
			const wchar_t* szPath = L"c:\\users\\tcadmin\\creo test\\test.prt.1";
			CPathEx path1(szPath);
			Assert::IsTrue(path1.GetParentPath().compare(L"c:\\users\\tcadmin\\creo test") == 0, path1.GetParentPath().c_str());
			Assert::IsTrue(path1.GetFileName().compare(L"test.prt") == 0, path1.GetFileName().c_str());
			Assert::IsTrue(path1.GetExtension().compare(L".1") == 0, path1.GetExtension().c_str());
			const wchar_t* szFolder = L"C:\\Users\\tcadmin\\AppData\\Local\\";	
		
			Assert::IsTrue(CPathEx::IsDir(szFolder), L"It is a folder");
			Assert::IsFalse(CPathEx::IsFile(szFolder), L"It is not a file");
		}

		TEST_METHOD(TestRegMatch)
		{
			UTEST_LOG("TestRegMatch:");
			/*static std::wstring g_exportFileNamePatterns[] = {
				wstring(L"(.*)(_cpy_\\d{1}.igs)"),
				wstring(L"(.*)(.igs)"),
				wstring(L".neu$"),
				wstring(L"_cpy_\\d{1}.vda$"),
				wstring(L"_(asm|prt).wrl$")
			};

			wstring f1(L"10p2_prt_cp1_cpy_1.igs");
			std::wregex reg1(L"(.*)(_cpy_\\d{1}.igs)", std::regex::ECMAScript | std::regex::icase);
			std::wsmatch m;
			Assert::IsTrue(std::regex_match(f1, m, reg1), L"igs not matched");
			Logger::WriteMessage(m[1].str().c_str());*/

		}

		TEST_METHOD(TestGetHostName)
		{
			UTEST_LOG("TestGetHostName:");
			auto host = RMXUtils::GetHostName();
			UTEST_LOG(host.c_str());
			Assert::IsTrue(_wcsicmp(host.c_str(), L"tc19cl01") == 0, L"Wrong host name");		
		}

		TEST_METHOD(TestReplaceString)
		{
			UTEST_LOG("TestReplaceString:");
			wstring text(L"TestA $(Host) TestB");
			RMXUtils::IReplaceAllString<wchar_t>(text, wstring(L"$(hOst)"), wstring(L"Hello"));
			Assert::IsTrue(_wcsicmp(text.c_str(), L"Testa hello testb") == 0, L"Wrong ReplayAllString");
			UTEST_LOG(text.c_str());
		}

		TEST_METHOD(TestCompareTags)
		{
			UTEST_LOG("TestCompareTags:");
			string tags1("{\"ip_classification\" : [\"super-secret\"], \"gov_classification\":[\"aaa\"]}");
			//string tags1("{\"gov_classification\":[\"aaa\",\"test1\",\"ccc\",\"123next!\"],\"ip_classification\":[\"super-secret\"]}");
			string tags2("{\"gov_classification\":[\"aaa\"], \"ip_classification\" : [\"super-secret\"]}");
			TTagList tagList1;
			RMXLib::CTagCollection tagColl(tags1);
			tagList1 = tagColl.toList();
			for (auto & tag : tagList1)
			{
				std::sort(tag.second.begin(), tag.second.end(), TagKeyComp());
			}

			TTagList tagList2;
			RMXLib::CTagCollection tagColl2(tags2);
			tagList2 = tagColl2.toList();
			for (auto & tag : tagList2)
			{
				std::sort(tag.second.begin(), tag.second.end(), TagKeyComp());
			}

			Assert::IsTrue(tagList1.size() == tagList2.size() &&
				std::equal(tagList1.begin(), tagList1.end(), tagList2.begin()), L"Compare tags failed");
		}

		TEST_METHOD(TestGetCreoInstallDir)
		{
			const wchar_t* szRegCreoRMX = L"SOFTWARE\\NextLabs\\CreoRMX";
			const wchar_t* szRegValueName = L"CreoDir";
			CPathEx  installDir;
			HKEY hSubKey;
			LONG retResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegCreoRMX, 0, KEY_WOW64_64KEY|KEY_READ, &hSubKey);
			if (ERROR_SUCCESS == retResult)
			{
				std::vector<wchar_t> data;
				DWORD cbData = 0;
				retResult = RegQueryValueEx(hSubKey, szRegValueName, 0, nullptr, nullptr, &cbData);
				if (ERROR_SUCCESS == retResult)
				{
					data.resize(cbData / sizeof(wchar_t));
					DWORD dwType = REG_SZ;
					retResult = RegQueryValueEx(hSubKey, szRegValueName, 0, &dwType, (LPBYTE)&data[0], &cbData);
					if (ERROR_SUCCESS == retResult)
					{
						data.resize(cbData / sizeof(wchar_t));
						Assert::IsTrue(!data.empty(), L"CreoDir key not found in registry SOFTWARE\\NextLabs\\CreoRMX");						
					}
				}
			}

		}
};
}