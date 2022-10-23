#include "uTestInc.h"

#include <string>
#include <regex>
#include <experimental/filesystem>
#include <windows.h>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include "..\LibRMX\LibRMX.h"
#include <iostream>
#include <RMXUtils.h>

namespace fs = std::experimental::filesystem;
using namespace std;

#define __TESTDATADIR__ _W(_STRINGIZE(TESTDATA_DIR));
#define FILE_UNPROTECT L"unprotected.txt"
#define FILE_PROJECTED L"protected.txt.nxl"

#define FILE_UNPROTECTEDWITHOUTEXT L"unprotected"
#define FILE_PROJECTEDWITHOUTEXT L"protected.txt"
static wstring g_testDataDir;

namespace RMXUTest
{
	TEST_CLASS(CLibRMXUTest)
	{
	private:
		wstring createMethodRPMDir(const wstring& methodName)
		{
			fs::path folderPath(g_testDataDir + methodName); 
			fs::create_directories(folderPath); 
			fs::copy(g_testDataDir, folderPath.wstring()); 
			bool ret = RMX_AddRPMDir(const_cast<wchar_t*>(folderPath.c_str())); 
			Assert::IsTrue(ret, L"AddRPMDir not working"); 
			UTEST_LOG_FMT(L"Method RPMDir : %s", folderPath.c_str());
			return  folderPath.wstring(); 
		}

#define METHOD_RPMDIR() createMethodRPMDir(RMXUtils::s2ws(__func__))

	public:
		TEST_CLASS_INITIALIZE(LibRMXUTest_Class_Init)
		{
			Logger::WriteMessage("In LibRMXUTest Class Initialize");
			RMX_Initialize();
			// HACK format
			wstring srcDataDir = __TESTDATADIR__;
			srcDataDir.erase(0, 1); // erase the first quote
			srcDataDir.erase(srcDataDir.length() - 2); // erase the last quote and the dot

			std::wstring currTime;
			std::time_t tNow = std::time(nullptr);
			wchar_t wstr[20];
			if (std::wcsftime(wstr, 20, L"%Y%m%d%H%M%S", std::localtime(&tNow))) {
				currTime.assign(wstr);
			}

			g_testDataDir = fs::temp_directory_path().wstring() + L"LibRMXTest_" + currTime + L"\\";
			fs::create_directories(g_testDataDir);
			fs::copy(srcDataDir, g_testDataDir);

			Logger::WriteMessage("LibRMXTest working folder: ");
			Logger::WriteMessage(g_testDataDir.c_str());
		}

		TEST_CLASS_CLEANUP(LibRMXUTest_Class_Clearup)
		{
			Logger::WriteMessage("In LibRMXUTest Class Cleanup");
			// Clear all rpm folders
			fs::path rootPath(g_testDataDir);
			for (auto& p : fs::directory_iterator(rootPath))
			{
				if (!p.path().has_extension())
					RMX_RemoveRPMDir(const_cast<wchar_t*>(p.path().c_str()));
			}
			try
			{
				error_code ec;
				if (fs::remove_all(g_testDataDir, ec) == static_cast<std::uintmax_t>(-1))
					Logger::WriteMessage("Failed to remove test folder");
			}
			catch (...)
			{
				Logger::WriteMessage("Exception to remove test folder");
			}
			RMX_Terminate();
		}
		TEST_METHOD_INITIALIZE(CLibRMXUTest_Method_Init)
		{
			Logger::WriteMessage("In RMXTest Method Initialize");
		}

		TEST_METHOD_CLEANUP(CLibRMXUTest_Method_Cleanup)
		{
			Logger::WriteMessage("In RMXTest Method Cleanup");
		}

		/*TEST_METHOD(TestProtectAllFilesInFolder)
		{
			WIN32_FIND_DATA findData;
			wstring searchDir(L"C:\\Source\\Docs\\EBSolution\\bicycle\\*");
			HANDLE hFind = FindFirstFile(searchDir.c_str(), &findData);
			if (hFind == INVALID_HANDLE_VALUE)
			{
				Assert::IsTrue(false, L"TestProtectAllFilesInFolder");
			}
			do
			{
				if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					wstring srcFile = L"C:\\Source\\Docs\\EBSolution\\bicycle\\";
					srcFile += findData.cFileName;
					wstring destFile = L"C:\\Source\\Docs\\EBSolution\\bicycle_protected_test\\";
					destFile += findData.cFileName;
					RMX_ProtectFile(srcFile.c_str(), "{\"ip_classification\":\"secret\"}", destFile.c_str());
					
				}
			} while (FindNextFile(hFind, &findData));
			FindClose(hFind);
		}
*/
		TEST_METHOD(TestProtectFile)
		{
			UTEST_LOG_ENTER
			fs::path folderPath(METHOD_RPMDIR());
			wstring filePath = folderPath / FILE_UNPROTECT;
			wstring destPath = folderPath / L"newProtected.txt.nxl";
			
			Logger::WriteMessage(filePath.c_str());

			bool ret = RMX_ProtectFile(filePath.c_str(), "{\"ip_classification\":\"secret\"}", destPath.c_str());

			Assert::IsTrue(ret, L"Protection file failed");
			Assert::IsTrue(RMX_IsProtectedFile(destPath.c_str()), L"No protect file is generated");
		}

		/*TEST_METHOD(TestReProtectFile)
		{
			Logger::WriteMessage("TestReProtectFile:");
			wstring filePath = g_testDataDir + FILE_UNPROTECT;
			wstring destPath = g_testDataDir + L"TestReProtectFile\\" + FILE_PROJECTED;

			Logger::WriteMessage(filePath.c_str());

			char* tags = "{\"ip_classification\":[\"secret\"], \"gov_classification\":[\"aaa\"]}";
			bool ret = RMX_ProtectFile(filePath.c_str(), tags, destPath.c_str());

			Assert::IsTrue(ret, L"Protection file failed");
			Assert::IsTrue(RMX_IsProtectedFile(destPath.c_str()), L"No protect file is generated");

			ret = RMX_ReProtectFile(destPath.c_str());
			Assert::IsTrue(ret, L"Reprotect file failed");
			Assert::IsTrue(RMX_IsProtectedFile(destPath.c_str()), L"No protect file is generated");
		}*/

		TEST_METHOD(TestIsProtectedFile)
		{
			UTEST_LOG_ENTER
			wstring filePath = g_testDataDir + FILE_PROJECTED;

			Logger::WriteMessage(filePath.c_str());

			bool ret = RMX_IsProtectedFile(filePath.c_str());
			Assert::IsTrue(ret, L"IsProtectedFile not working");
		}

		TEST_METHOD(TestIsProtectedFileInRPM)
		{
			UTEST_LOG_ENTER
			wstring destFolder = g_testDataDir + L"TestIsProtectedFileInRPM";
			fs::create_directories(destFolder);
			bool ret;
			ret = RMX_AddRPMDir(const_cast<wchar_t*>(destFolder.c_str()));
			Assert::IsTrue(ret, L"AddRPMDir not working");

			wstring srcFile = g_testDataDir + FILE_PROJECTED;
			wstring destFile = destFolder + L"\\" + FILE_PROJECTED;
			Assert::IsTrue(RMX_CopyNxlFile(srcFile.c_str(), destFile.c_str(), false), L"1.Failed to copy nxl file");
			ret = RMX_IsProtectedFile(destFile.c_str());
			Assert::IsTrue(ret, L"1.IsProtectedFile not working");

			wstring destFile2 = destFolder + L"\\" + FILE_PROJECTEDWITHOUTEXT;
			Assert::IsTrue(RMX_CopyNxlFile(srcFile.c_str(), destFile2.c_str(), false), L"2.Failed to copy nxl file");
			ret = RMX_IsProtectedFile(destFile2.c_str());
			Assert::IsTrue(ret, L"2.IsProtectedFile not working");
		}

		TEST_METHOD(TestAddRPMFolder)
		{
			UTEST_LOG_ENTER
			bool ret;
			fs::path subFolderPath(g_testDataDir + L"TestAddRPMFolder_sub");
			fs::create_directories(subFolderPath);
			ret = RMX_AddRPMDir(const_cast<wchar_t*>(subFolderPath.c_str()));
			Assert::IsTrue(ret, L"AddRPMDir not working");

			ret = RMX_IsRPMFolder(const_cast<wchar_t*>(subFolderPath.c_str()));
			Assert::IsTrue(ret, L"Failed to AddRPMDir ");

			ret = RMX_IsRPMFolder(const_cast<wchar_t*>(g_testDataDir.c_str()));
			Assert::IsFalse(ret, L"Parent folder is not RPM folder ");

			RMX_SAFEDIRRELATION dirRel;
			Assert::IsTrue(RMX_GetSafeDirRelation(const_cast<wchar_t*>(g_testDataDir.c_str()), dirRel));
			Assert::IsTrue(dirRel == RMX_AncestorOfSafeDir, L"Ancestor of safe dir");

			Assert::IsTrue(RMX_GetSafeDirRelation(const_cast<wchar_t*>(subFolderPath.c_str()), dirRel));
			Assert::IsTrue(dirRel == RMX_IsSafeDir, L"Safe dir");

			fs::path subFolderPath2(g_testDataDir + L"TestAddRPMFolder_normal");
			fs::create_directories(subFolderPath2);
			ret = RMX_AddRPMDir2(const_cast<wchar_t*>(subFolderPath2.c_str()), RMX_RPMFOLDER_NORMAL);
			Assert::IsTrue(ret, L"AddRPMDir not working");

			ret = RMX_IsRPMFolder(const_cast<wchar_t*>(subFolderPath2.c_str()));
			Assert::IsTrue(ret, L"Failed to AddRPMDir ");

			fs::path subFolderPath3(g_testDataDir + L"TestAddRPMFolder_api");
			fs::create_directories(subFolderPath3);
			ret = RMX_AddRPMDir2(const_cast<wchar_t*>(subFolderPath3.c_str()), RMX_RPMFOLDER_API);
			Assert::IsTrue(ret, L"AddRPMDir not working");

			ret = RMX_IsRPMFolder(const_cast<wchar_t*>(subFolderPath3.c_str()));
			Assert::IsTrue(ret, L"Failed to AddRPMDir ");


		}

		TEST_METHOD(TestProtectFileInRPMFolder)
		{
			UTEST_LOG_ENTER
			fs::path folderPath(METHOD_RPMDIR());
			wstring filePath = folderPath / FILE_UNPROTECT;
			wstring destPath = folderPath / L"TestProtectFileInRPMFolder.txt.nxl";

			Logger::WriteMessage(filePath.c_str());

			bool ret = RMX_ProtectFile(filePath.c_str(), nullptr, destPath.c_str());
			Assert::IsTrue(ret, L"Protection file failed");
			Assert::IsTrue(RMX_IsProtectedFile(destPath.c_str()), L"No protect file is generated");
		}

		/*TEST_METHOD(TestReProtectFileInRPMFolder)
		{
			UTEST_LOG_ENTER
			fs::path folderPath(METHOD_RPMDIR());
			wstring filePath = folderPath / FILE_UNPROTECT;
			wstring destPath = folderPath / L"\\TestReProtectFileInRPMFolder.txt.nxl";
			UTEST_LOG(filePath.c_str());

			char* tags = "{\"ip_classification\":[\"secret\"], \"gov_classification\":[\"aaa\"]}";
			bool ret = RMX_ProtectFile(filePath.c_str(), tags, destPath.c_str());
			Assert::IsTrue(ret, L"Protection file failed");
			Assert::IsTrue(RMX_IsProtectedFile(destPath.c_str()), L"No protect file is generated");

			ret = RMX_ReProtectFile(destPath.c_str());
			Assert::IsFalse(ret, L"Reprotect in RPM dir not supported.");
			
		}*/

		TEST_METHOD(TestRPMEditSaveInRPMFolder)
		{
			UTEST_LOG_ENTER
			fs::path folderPath(METHOD_RPMDIR());
			wstring filePath = folderPath / FILE_UNPROTECT;
			wstring destPath = folderPath / L"\\TestRPMEditSaveInRPMFolder.txt.nxl";
			UTEST_LOG(filePath.c_str());

			char* tags = "{\"ip_classification\":[\"secret\"], \"gov_classification\":[\"aaa\"]}";
			bool ret = RMX_ProtectFile(filePath.c_str(), tags, destPath.c_str());
			Assert::IsTrue(ret, L"Protection file failed");
			Assert::IsTrue(RMX_IsProtectedFile(destPath.c_str()), L"No protect file is generated");

			ret = RMX_RPMEditSaveFile(destPath.c_str(), nullptr, false, RMX_EditSaveMode_SaveAndExit);
			Assert::IsTrue(ret, L"RMX_RPMEditSaveFile failed.");

		}
		TEST_METHOD(TestCopyNxlFileFromRPM)
		{
			UTEST_LOG_ENTER
			bool ret;
			wstring wstrSrcFolder = g_testDataDir + L"TestCopyNxlFileFromRPM_From";
			fs::create_directories(wstrSrcFolder);
			fs::copy(g_testDataDir, wstrSrcFolder);
			ret = RMX_AddRPMDir(const_cast<wchar_t*>(wstrSrcFolder.c_str()));
			Assert::IsTrue(ret, L"AddRPMDir not working");

			// Case1: Copy file with nxl extension
			wstring wstrSrcFile(wstrSrcFolder + L"\\" + FILE_PROJECTED);
			wstring wstrDestFile = g_testDataDir + L"TestCopyNxlFileFromRPM_To1\\" + FILE_PROJECTED;
			Assert::IsTrue(RMX_CopyNxlFile(wstrSrcFile.c_str(), wstrDestFile.c_str(), false), L"1.Failed to copy nxl file");
			Assert::IsTrue(RMX_IsProtectedFile(wstrDestFile.c_str()), L"1.File lost protection");

			// Case2: Copy file without nxl extension
			// Test copy nxl file back to RMP, the original unprotected file with same name still exists.
			wstrSrcFile = wstrSrcFolder + +L"\\" + FILE_PROJECTEDWITHOUTEXT;
			wstrDestFile = g_testDataDir + L"TestCopyNxlFileFromRPM_To2\\" + FILE_PROJECTED;
			Assert::IsTrue(RMX_CopyNxlFile(wstrSrcFile.c_str(), wstrDestFile.c_str(), false), L"2.Failed to copy nxl file");
			Assert::IsTrue(RMX_IsProtectedFile(wstrDestFile.c_str()), L"2.File lost protection");

			// Case3: Copy file from sub folder of a rpm folder
			wstrSrcFolder += L"\\PosterityOfSafeDir";
			fs::create_directories(wstrSrcFolder);
			fs::copy(g_testDataDir, wstrSrcFolder);
			wstrSrcFile = wstrSrcFolder + +L"\\" + FILE_PROJECTED;
			wstrDestFile = g_testDataDir + L"TestCopyNxlFileFromRPM_To3\\" + FILE_PROJECTED;
			Assert::IsTrue(RMX_CopyNxlFile(wstrSrcFile.c_str(), wstrDestFile.c_str(), false), L"3.Failed to copy nxl file");
			Assert::IsTrue(RMX_IsProtectedFile(wstrDestFile.c_str()), L"3.File lost protection");

		}

		TEST_METHOD(TestCopyNxlFileToRPM)
		{
			UTEST_LOG_ENTER
			wstring srcFolder = g_testDataDir + L"TestCopyNxlFileToRPM_From";
			fs::create_directories(srcFolder);
			fs::copy(g_testDataDir, srcFolder.c_str());

			wstring destFolder = g_testDataDir + L"TestCopyNxlFileToRPM_To";
			fs::create_directories(destFolder);
			bool ret;
			ret = RMX_AddRPMDir(const_cast<wchar_t*>(destFolder.c_str()));
			Assert::IsTrue(ret, L"AddRPMDir not working");
			
			// Case1: Copy file into rpm, a unprotected file with same name exists. 
			wstring unprotectFile = srcFolder + L"\\" + FILE_UNPROTECT;
			wstring unprotectFile2 = destFolder + L"\\" + FILE_PROJECTEDWITHOUTEXT;
			fs::copy_file(unprotectFile, unprotectFile2);
			wstring srcFile = srcFolder + L"\\" + FILE_PROJECTED;
			wstring destFile = destFolder + L"\\" + FILE_PROJECTEDWITHOUTEXT;
			Assert::IsTrue(RMX_CopyNxlFile(srcFile.c_str(), destFile.c_str(), false), L"1.Failed to copy nxl file");
			Assert::IsTrue(RMX_IsProtectedFile(destFile.c_str()), L"1.File lost protection");

			// Case2: Copy file into rpm
			destFile = destFolder + L"\\protected1.txt.nxl";
			Assert::IsTrue(RMX_CopyNxlFile(srcFile.c_str(), destFile.c_str(), false), L"2.Failed to copy nxl file");
			Assert::IsTrue(RMX_IsProtectedFile(destFile.c_str()), L"2.File lost protection");
		}

		TEST_METHOD(TestCopyNxlFileRPMToRPM)
		{
			UTEST_LOG_ENTER
			bool ret;
			wstring wstrSrcFolder = g_testDataDir + L"TestCopyNxlFileRPMToRPM_From";
			fs::create_directories(wstrSrcFolder);
			fs::copy(g_testDataDir, wstrSrcFolder);
			ret = RMX_AddRPMDir(const_cast<wchar_t*>(wstrSrcFolder.c_str()));
			Assert::IsTrue(ret, L"1. AddRPMDir not working");

			wstring destFolder = g_testDataDir + L"TestCopyNxlFileRPMToRPM_To";
			fs::create_directories(destFolder);
			ret = RMX_AddRPMDir(const_cast<wchar_t*>(destFolder.c_str()));
			Assert::IsTrue(ret, L"2. AddRPMDir not working");

			// Case1: Copy file with nxl extension
			wstring wstrSrcFile(wstrSrcFolder + L"\\" + FILE_PROJECTED);
			wstring wstrDestFile = destFolder + L"\\" + FILE_PROJECTED;
			Assert::IsTrue(RMX_CopyNxlFile(wstrSrcFile.c_str(), wstrDestFile.c_str(), false), L"1.Failed to copy nxl file");
			Assert::IsTrue(RMX_IsProtectedFile(wstrDestFile.c_str()), L"1.File lost protection");

			// Case2: Copy file without nxl extension
			wstrSrcFile = wstrSrcFolder + +L"\\" + FILE_PROJECTEDWITHOUTEXT;
			wstrDestFile = destFolder + L"\\" + FILE_PROJECTED;
			Assert::IsTrue(RMX_CopyNxlFile(wstrSrcFile.c_str(), wstrDestFile.c_str(), false), L"2.Failed to copy nxl file");
			Assert::IsTrue(RMX_IsProtectedFile(wstrDestFile.c_str()), L"2.File lost protection");
		}

		//TEST_METHOD(TestMisc)
		//{
		//	Logger::WriteMessage("TestMisc:");
		//	//wstring appPath = _W(_STRINGIZE(CREOAPPP_DIR));
		//	//appPath.erase(0, 1); // erase the first quote
		//	//appPath.erase(appPath.length() - 3); // erase the last quote and the dot
		//	//Assert::IsTrue(RMX_RPMResisterApp(appPath.c_str()), L"Failed to register app");
		//	char* tags = "{\"ip_classification\":[\"secret\"], \"gov_classification\":[\"aaa\"]}";
		//	RMX_ProtectFile(L"C:\\Users\\tcadmin\\creo3 test\\p1.prt.1", tags, L"C:\\Users\\tcadmin\\creo3 test\\p1.prt.2.nxl");
		//}

		TEST_METHOD(TestRemoveRPMFolder)
		{
			UTEST_LOG_ENTER

			wstring destFolder = g_testDataDir + L"TestRemoveRPMFolder rpm";
			fs::create_directories(destFolder);

			bool ret;
			ret = RMX_AddRPMDir(const_cast<wchar_t*>(destFolder.c_str()));
			Assert::IsTrue(ret, L"AddRPMDir not working");

			ret = RMX_RemoveRPMDir(const_cast<wchar_t*>(destFolder.c_str()));
			Assert::IsTrue(ret, L"RemoveRPMDir not working");

			ret = RMX_IsRPMFolder(destFolder.c_str());
			Assert::IsFalse(ret, L"Failed to remove RPM dir");
		}

		// Only need to perform once by installer.
		//TEST_METHOD(TestRegisterApp)
		//{
		//	Logger::WriteMessage(L"TestRegisterApp");
		//	wstring appPath = _W(_STRINGIZE(CREOAPPP_DIR));
		//	appPath.erase(0, 1); // erase the first quote
		//	appPath.erase(appPath.length() - 3); // erase the last quote and the dot
		//	Assert::IsTrue(RMX_RPMResisterApp(appPath.c_str()), L"Failed to register app");

		//	Assert::IsTrue(RMX_RPMUnresisterApp(appPath.c_str()), L"Failed to unregister app");
		//}

		TEST_METHOD(TestGetTags)
		{
			UTEST_LOG_ENTER
			wstring wstrSrcFile(g_testDataDir + FILE_PROJECTED);
			char* szTags = nullptr;
			RMX_GetTags(wstrSrcFile.c_str(), &szTags);
			Assert::IsNotNull(szTags);
			Logger::WriteMessage(szTags);
			Assert::IsTrue(strlen(szTags) != 0, L"Failed to get tags");
			RMX_ReleaseArray((void*)szTags);
		}

		TEST_METHOD(TestGetTagsFromRPM)
		{
			UTEST_LOG_ENTER
			wstring destFolder = g_testDataDir + L"TestGetTags FromRPM";
			fs::create_directories(destFolder);
			bool ret;
			ret = RMX_AddRPMDir(const_cast<wchar_t*>(destFolder.c_str()));
			Assert::IsTrue(ret, L"AddRPMDir not working");

			wstring srcFile = g_testDataDir + FILE_PROJECTED;
			wstring destFile = destFolder + L"\\" + FILE_PROJECTED;
			Assert::IsTrue(RMX_CopyNxlFile(srcFile.c_str(), destFile.c_str(), false), L"Failed to copy nxl file");
			
			char* szTags = nullptr;
			RMX_GetTags(destFile.c_str(), &szTags);
			Assert::IsNotNull(szTags);
			Logger::WriteMessage(szTags);
			Assert::IsTrue(strlen(szTags) != 0, L"Case1: Failed to get tags");
			RMX_ReleaseArray((void*)szTags);
		}

		TEST_METHOD(TestCheckRights)
		{
			UTEST_LOG_ENTER
			wstring wstrSrcFile(g_testDataDir + FILE_PROJECTED);
			Assert::IsTrue(RMX_CheckRights(wstrSrcFile.c_str(), RMX_RIGHT_EDIT, false), L"Failed to check RIGHT_EDIT");
			Assert::IsTrue(RMX_CheckRights(wstrSrcFile.c_str(), RMX_RIGHT_VIEW, false), L"Failed to check RIGHT_VIEW");
			Assert::IsFalse(RMX_CheckRights(wstrSrcFile.c_str(), RMX_RIGHT_CLASSIFY, false), L"Failed to check RMX_RIGHT_CLASSIFY");
		}

		TEST_METHOD(TestCheckRightsInRPM)
		{
			UTEST_LOG_ENTER
			wstring destFolder = g_testDataDir + L"TestCheckRightsInRPM";
			fs::create_directories(destFolder);
			

			wstring srcFile = g_testDataDir + FILE_PROJECTED;
			wstring destFile = destFolder + L"\\" + FILE_PROJECTED;
			Assert::IsTrue(RMX_CopyNxlFile(srcFile.c_str(), destFile.c_str(), false), L"Failed to copy nxl file");
			bool ret;
			ret = RMX_AddRPMDir(const_cast<wchar_t*>(destFolder.c_str()));
			Assert::IsTrue(ret, L"AddRPMDir not working");
			Assert::IsTrue(RMX_CheckRights(destFile.c_str(), RMX_RIGHT_EDIT, false), L"Failed to check RIGHT_EDIT");
			Assert::IsTrue(RMX_CheckRights(destFile.c_str(), RMX_RIGHT_VIEW, false), L"Failed to check RIGHT_VIEW");

			// Not able to get rights without nxl file extension
			/*wstring destFileNoNxlExt = destFolder + L"\\protected.txt";
			Assert::IsTrue(RMX_CopyNxlFile(srcFile.c_str(), destFileNoNxlExt.c_str(), true), L"Failed to copy nxl file");
			Assert::IsTrue(RMX_CheckRights(destFileNoNxlExt.c_str(), RMX_RIGHT_EDIT), L"Failed to check RIGHT_EDIT");
			Assert::IsTrue(RMX_CheckRights(destFileNoNxlExt.c_str(), RMX_RIGHT_VIEW), L"Failed to check RIGHT_VIEW");*/
		}

		// Call this test method to set up dev machine.
		TEST_METHOD(TestInitEnv)
		{
			UTEST_LOG_ENTER
			// Register creo apps
			vector<wstring> apps = { 
				L"C:\\Program Files\\PTC\\Creo 3.0\\M170\\Common Files\\x86e_win64\\obj\\xtop.exe",
				L"C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\Common7\\IDE\\CommonExtensions\\Microsoft\\TestWindow\\vstest.executionengine.exe",
				//L"C:\\Program Files\\PTC\\Creo 3.0\\M170\\Parametric\\bin\\parametric.exe"
				//L"C:\\Source\\Codes\\TeamcenterDCE_5.0\\CreoIntegration\\x64\\Debug\\rmxregister.exe"
			};
			for (auto app : apps)
				Assert::IsTrue(RMX_RPMResisterApp(app.c_str()), L"Failed to register app");

		//	// Add one rpm dir for test purpose.
		//	Assert::IsTrue(RMX_AddRPMDir(L"C:\\Users\\tcadmin\\Creo"));
		}

		TEST_METHOD(TestAddTrustedProcess)
		{
			UTEST_LOG_ENTER
			STARTUPINFO si;
			si.dwFlags = STARTF_USESHOWWINDOW;
			si.wShowWindow = FALSE;
			PROCESS_INFORMATION pi;
			wstring appPath = L"C:\\Windows\\System32\\notepad.exe";

			wstring destFolder = g_testDataDir + L"TestAddTrustedProcess_rmp";
			fs::create_directories(destFolder);
			bool ret;
			ret = RMX_AddRPMDir(const_cast<wchar_t*>(destFolder.c_str()));
			Assert::IsTrue(ret, L"AddRPMDir not working");
			fs::copy(g_testDataDir, destFolder.c_str());
			wstring filePath = destFolder + L"\\" + FILE_PROJECTED;
			
			WCHAR wzCmdLine[MAX_PATH];
			wsprintfW(wzCmdLine, L"%s %s", appPath.c_str(), filePath.c_str());
			ZeroMemory(&si, sizeof(si));
			ZeroMemory(&pi, sizeof(pi));

			DWORD dwExitCode = 0;
			if (CreateProcessW(
				NULL,
				wzCmdLine,
				NULL,
				NULL,
				false,
				CREATE_SUSPENDED,
				NULL,
				NULL,
				&si,
				&pi))
			{
				Assert::IsTrue(RMX_RPMAddTrustedProcess(pi.dwProcessId), L"Failed to add trusted process");
				Assert::IsFalse(ResumeThread(pi.hThread) == -1, L"Failed to resume thread");
				TerminateProcess(pi.hProcess, 0);

				const DWORD result = WaitForSingleObject(pi.hProcess, 50);
				if (result == WAIT_OBJECT_0) {
					// Success
				}
				else {
					// Timed out or an error occurred
				}

				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
		}

		TEST_METHOD(TestRMXLastError)
		{
			UTEST_LOG_ENTER
			wstring destFolder = g_testDataDir + L"TestRMXLastError";
			fs::create_directories(destFolder);
			//bool ret;
			//ret = RMX_AddRPMDir(const_cast<wchar_t*>(destFolder.c_str()));
			//Assert::IsTrue(ret, L"AddRPMDir not working");

			wstring srcFile = g_testDataDir + FILE_PROJECTED;
			wstring destFile = destFolder + L"\\" + FILE_PROJECTEDWITHOUTEXT;
			Assert::IsTrue(CopyFile(srcFile.c_str(), destFile.c_str(), false) != 0, L"1.Failed to copy file");
			Assert::IsTrue(RMX_IsProtectedFile(destFile.c_str()), L"1.IsProtectedFile not working");
			Assert::IsFalse(RMX_IsValidNxl(destFile.c_str()), L"1.IsValidNxl not working");
			wchar_t* szErrMsg = nullptr;
			Assert::IsTrue(RMX_GetLastError(&szErrMsg) == RMX_ERROR_INVALID_NXLFILE, L"unexpected RMX last error");
			Assert::IsTrue(szErrMsg && (wcslen(szErrMsg) > 0), L"Invalid error message");
			Logger::WriteMessage(szErrMsg);
			RMX_ReleaseArray((void*)szErrMsg);
		}

		TEST_METHOD(TestMergeTags)
		{
			UTEST_LOG_ENTER
			char* tags = "{ \"ref_types\" : [\"2\", \"2\", \"2\"], \"last_mod_user\" : [\"infodba test (infodba)\"], \"IP_classification\":[\"SEcret\"], \"gov_classification\" : [\"aaa\", \"jin1\", \"jin2\"] } {\"ip_classification\":[\"top-secret\"], \"gov_classification\" : [\"jin1\", \"bbb\"] }{\"revision_number\":[\"1\"], \"last_mod_date\" : [\"22-Aug-2019 12:14\"], \"last_mod_user\" : [\"infodba (infodba)\"], \"gov_classification\" : [\"jin2\"], \"ip_classification\" : [\"secret\"], \"ref_types\" : [\"2\", \"2\", \"2\"], \"ref_names\" : [\"MetaData\", \"PrtFile\", \"JPEG\"]}";
			char* newTags = nullptr;
			Assert::IsTrue(RMX_MergeTags(tags, &newTags), L"Failed to merge tags");
			if (newTags != nullptr)
			{
				Logger::WriteMessage(newTags);
				RMX_ReleaseArray((void*)newTags);
			}
		}

		TEST_METHOD(TestGetActivitiyInfo)
		{
			UTEST_LOG_ENTER
			wstring destFolder = g_testDataDir + L"TestGetActivitiyInfo";
			fs::create_directories(destFolder);


			wstring srcFile = g_testDataDir + FILE_PROJECTED;
			wstring destFile = destFolder + L"\\" + FILE_PROJECTED;
			Assert::IsTrue(RMX_CopyNxlFile(srcFile.c_str(), destFile.c_str(), false), L"1.Failed to copy nxl file");

			auto ret = RMX_AddActivityLog(destFile.c_str(), RMX_OPrint, RMX_RDenied);		
			Assert::IsTrue(ret, L"Failed to RMX_AddActivityLog 1");

			ret = RMX_AddActivityLog(destFile.c_str(), RMX_ODownload, RMX_RAllowed);
			Assert::IsTrue(ret, L"Failed to RMX_AddActivityLog 2");
		}
};
}