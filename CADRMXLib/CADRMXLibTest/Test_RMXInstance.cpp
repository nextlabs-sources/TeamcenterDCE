#include "stdafx.h"

namespace RMXLTest
{
	TEST_CLASS(RMXInstanceTest)
	{	
	private:
		IRMXInstance* m_pInst;
		IRMXUser* m_pUser;

		RMXLTestSession m_rmxSess;

	public:
		TEST_CLASS_INITIALIZE(RMXInstanceTest_Class_Init)
		{
			UTEST_LOG_ENTER;		
		}

		TEST_CLASS_CLEANUP(RMXInstanceTest_Class_Clearup)
		{
			UTEST_LOG_ENTER;
		}

		TEST_METHOD_INITIALIZE(RMXInstanceTest_Method_Init)
		{
			UTEST_LOG_ENTER;
			m_rmxSess.InitRMXSession();
			m_pInst = m_rmxSess.Inst();
			m_pUser = m_rmxSess.User();
			Assert::IsTrue(m_pInst != nullptr && m_pUser != nullptr, L"InitRMXSession failed");
		}

		TEST_METHOD_CLEANUP(RMXInstanceTest_Method_Cleanup)
		{
			UTEST_LOG_ENTER;
			m_rmxSess.TerminateRMXSession();
		}

		TEST_METHOD(TestRegisterApp)
		{
			UTEST_LOG_ENTER;

			wchar_t szProcessName[MAX_PATH];
			Assert::IsFalse(GetModuleFileName(NULL, &szProcessName[0], MAX_PATH) == 0);
			UTEST_LOG(szProcessName);
			
			auto ret = m_pInst->RegisterApp(szProcessName);
			ASSERT_RMXERROR(!ret, ret, L"RegisterApp failed");
		}	

		TEST_METHOD(TestUnregisterApp)
		{
			UTEST_LOG_ENTER;

			wchar_t szProcessName[MAX_PATH];
			Assert::IsFalse(GetModuleFileName(NULL, &szProcessName[0], MAX_PATH) == 0);
			UTEST_LOG(szProcessName);

			auto ret = m_pInst->RegisterApp(szProcessName);
			ASSERT_RMXERROR(!ret, ret, L"RegisterApp failed");

			ret = m_pInst->UnregisterApp(szProcessName);
			ASSERT_RMXERROR(!ret, ret, L"UnregisterApp failed");
		}

		TEST_METHOD(TestIsAppRegistered)
		{
			UTEST_LOG_ENTER;

			wchar_t szProcessName[MAX_PATH];
			Assert::IsFalse(GetModuleFileName(NULL, &szProcessName[0], MAX_PATH) == 0);
			UTEST_LOG(szProcessName);
			bool registered;
			auto ret = m_pInst->IsAppRegistered(szProcessName, registered);
			ASSERT_RMXERROR(ret && registered, ret, L"IsAppRegistered failed");
		}
		 
		TEST_METHOD(TestAddTrustedApp)
		{
			UTEST_LOG_ENTER;
			
			auto pInst = m_rmxSess.RunRMXSession();;
			auto ret = pInst->AddTrustedApp(L"C:\\Windows\\System32\\notepad.exe");
			ASSERT_RMXERROR(!ret, ret, L"AddTrustedApp failed");
		}

		TEST_METHOD(TestRemoveTrustedApp)
		{
			UTEST_LOG_ENTER;

			auto pInst = m_rmxSess.RunRMXSession();;
			auto ret = pInst->RemoveTrustedApp(L"C:\\Windows\\System32\\notepad.exe");
			ASSERT_RMXERROR(!ret, ret, L"RemoveTrustedApp failed");
		}

		TEST_METHOD(TestAddRPMDir)
		{
			UTEST_LOG_ENTER;
			const wstring& tempDir = m_rmxSess.GenerateTempDir();
			//// Case1. invalid path
			auto ret = m_pInst->AddRPMDir(tempDir.c_str(), RMX_RPMFOLDER_API);
			ASSERT_RMXERROR(ret, ret, L"AddRPMDir- Expected failure for invalid path");

			// Case2: Valid path
			fs::create_directories(tempDir);
			auto ret2 = m_pInst->AddRPMDir(tempDir.c_str(), RMX_RPMFOLDER_API);
			ASSERT_RMXERROR(!ret2, ret2, L"AddRPMDir failed");
		}

		TEST_METHOD(TestRemoveRPMDir)
		{
			UTEST_LOG_ENTER;
			const wstring& tempDir = m_rmxSess.GenerateTempDir();
			
			// Case1. invalid path
			auto ret = m_pInst->RemoveRPMDir(tempDir.c_str(), true);
			ASSERT_RMXERROR(ret, ret, L"RemoveRPMDir- Expected failure for invalid path");
		
			// Case2: Valid path
			fs::create_directories(tempDir);
			auto ret2 = m_pInst->AddRPMDir(tempDir.c_str(), 1);
			ASSERT_RMXERROR(!ret2, ret2, L"AddRPMDir failed");
	
			ret2 = m_pInst->RemoveRPMDir(tempDir.c_str(), true);
			ASSERT_RMXERROR(!ret2, ret2, L"RemoveRPMDir failed");
		}

		TEST_METHOD(TestIsRPMFolder)
		{
			UTEST_LOG_ENTER;
			const wstring& tempDir = m_rmxSess.GenerateTempDir();
			// Case1. invalid path
			bool isRPMDir = false;
			RMX_RPMFolderRelation rel = RMX_NonRPMFolder;
			auto ret = m_pInst->IsRPMFolder(tempDir.c_str(), isRPMDir, &rel);
			ASSERT_RMXERROR(ret || isRPMDir || rel != RMX_NonRPMFolder, ret, L"IsRPMFolder- Expected failure for invalid path");
		
			// Case2: Valid path
			fs::create_directories(tempDir);
			ret = m_pInst->AddRPMDir(tempDir.c_str());
			ASSERT_RMXERROR(!ret, ret, L"AddRPMDir failed");
			ret = m_pInst->IsRPMFolder(tempDir.c_str(), isRPMDir, &rel);
			ASSERT_RMXERROR(!ret, ret, L"IsRPMFolder failed");
		}

		TEST_METHOD(TestEditCopyFile)
		{
			UTEST_LOG_ENTER;

			m_rmxSess.RunRMXSession();
			IRMXUser* pUser = m_rmxSess.User();
			IRMXInstance* pInst = m_rmxSess.Inst();

			// Prepare nxl file in non rpm dir
			const wstring& tmpDir = m_rmxSess.GenerateTempDir();
			const wstring& testFile = m_rmxSess.CreaeteDummyTxt(tmpDir);
			std::wstring newNXLFile;
			const char* tags = "{\"ip_classification\":[\"secret\"]}";
			RMXResult ret = pUser->ProtectFile(testFile, tmpDir, tags, newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"TestEditCopyFile: ProtectFile  failed");

			bool isProtected = false;
			RMX_RPMFolderRelation dirStatus;
			ret = pInst->GetFileStatus(newNXLFile, isProtected, &dirStatus);
			ASSERT_RMXERROR(!ret || !isProtected || dirStatus != RMX_NonRPMFolder, ret, L"TestEditCopyFile: GetFileStatus-0 returns wrong ");

			// Case1. Copy to RPM secret folder
			wstring copyFile;
			ret = pInst->EditCopyFile(newNXLFile, copyFile);
			ASSERT_RMXERROR(!ret || copyFile.empty(), ret, L"TestEditCopyFile: EditCopyFile-1 failed");
			
			isProtected = false;
			dirStatus = RMX_NonRPMFolder;
			ret = pInst->GetFileStatus(copyFile, isProtected, &dirStatus);
			ASSERT_RMXERROR(!ret || !isProtected || dirStatus != RMX_RPMInheritedFolder, ret, L"TestEditCopyFile: GetFileStatus-1 returns wrong ");

			// Case2: Copy to a specified RPM folder
			const wstring& rpmDir = m_rmxSess.GenerateTempDir();

			fs::create_directories(rpmDir);
			ret = pInst->AddRPMDir(rpmDir);
			ASSERT_RMXERROR(!ret, ret, L"TestEditCopyFile: AddRPMDir  failed");

			wstring copyFile2(rpmDir);
			ret = pInst->EditCopyFile(newNXLFile, copyFile2);
			ASSERT_RMXERROR(!ret || copyFile2.empty(), ret, L"TestEditCopyFile: EditCopyFile-2 failed");

			isProtected = false;
			dirStatus = RMX_NonRPMFolder;
			ret = pInst->GetFileStatus(copyFile2, isProtected, &dirStatus);
			ASSERT_RMXERROR(!ret || !isProtected || dirStatus != RMX_RPMInheritedFolder, ret, L"TestEditCopyFile: GetFileStatus-2 returns wrong ");
		}

		TEST_METHOD(TestEditSaveFile)
		{
			UTEST_LOG_ENTER;

			m_rmxSess.RunRMXSession();
			IRMXUser* pUser = m_rmxSess.User();
			IRMXInstance* pInst = m_rmxSess.Inst();

			// Prepare nxl file in rpm dir
			const wstring& tmpDir = m_rmxSess.GenerateTempDir();
			const wstring& testFile = m_rmxSess.CreaeteDummyTxt(tmpDir);
			auto ret = pInst->AddRPMDir(tmpDir);
			ASSERT_RMXERROR(!ret, ret, L"TestEditSaveFile: AddRPMDir  failed");

			std::wstring newNXLFile;
			const char* tags = "{\"ip_classification\":[\"secret\"]}";
			ret = pUser->ProtectFile(testFile, tmpDir, tags, newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"TestEditSaveFile: ProtectFile  failed");
			
			wstring nxlFile = testFile + L".nxl";
			ret = pInst->RPMCopyFile(newNXLFile, nxlFile, false);
			ASSERT_RMXERROR(!ret, ret, L"TestEditSaveFile: RPMCopyFile failed");

			bool isProtected = false;
			RMX_RPMFolderRelation dirStatus;
			ret = pInst->GetFileStatus(testFile, isProtected, &dirStatus);
			ASSERT_RMXERROR(!ret || !isProtected || dirStatus != RMX_RPMFolder, ret, L"TestEditSaveFile: GetFileStatus-0 returns wrong ");

			std::wfstream f;
			f.open(testFile, std::wfstream::in | std::wfstream::out | std::wfstream::app);
			if (f.is_open())
				f << L"this is editing";
			f.close();
			// Case1. Save back to rpm folder
			ret = pInst->EditSaveFile(testFile, L"", false, 3);
			ASSERT_RMXERROR(!ret, ret, L"TestEditSaveFile: EditSaveFile-1 failed");
			
			std::wifstream ifs(testFile);
			std::wstring content((std::istreambuf_iterator<wchar_t>(ifs)),
				(std::istreambuf_iterator<wchar_t>()));
			Assert::IsTrue(content.find(L"this is editing") != wstring::npos, L"TestEditSaveFile: Change lost");

			const wstring& copyDir = m_rmxSess.GenerateTempDir();
			fs::create_directories(copyDir);
			ret = pInst->AddRPMDir(copyDir);
			ASSERT_RMXERROR(!ret, ret, L"TestEditSaveFile: AddRPMDir-2  failed");

			wstring copyFile = copyDir + L"test.txt.nxl";
			ret = pInst->RPMCopyFile(testFile, copyFile);
			ASSERT_RMXERROR(!ret, ret, L"EditSaveFile: RPMCopyFile ");
		
			isProtected = false;
			dirStatus = RMX_NonRPMFolder;
			ret = pInst->GetFileStatus(copyFile, isProtected, &dirStatus);
			ASSERT_RMXERROR(!ret || !isProtected || dirStatus != RMX_RPMFolder, ret, L"EditSaveFile: GetFileStatus-1 returns wrong ");	

			std::wifstream ifs2(copyFile);
			std::wstring content2((std::istreambuf_iterator<wchar_t>(ifs2)),
				(std::istreambuf_iterator<wchar_t>()));
			Assert::IsTrue(content2.find(L"this is editing") != wstring::npos, L"TestEditSaveFile: 2.Change lost");

		}

		TEST_METHOD(TestRPMCopyFile)
		{
			UTEST_LOG_ENTER;

			m_rmxSess.RunRMXSession();
			IRMXUser* pUser = m_rmxSess.User();
			IRMXInstance* pInst = m_rmxSess.Inst();

			// Prepare nxl file in rpm dir
			const wstring& tmpDir = m_rmxSess.GenerateTempDir();
			const wstring& testFile = m_rmxSess.CreaeteDummyTxt(tmpDir);
			auto ret = pInst->AddRPMDir(tmpDir);
			ASSERT_RMXERROR(!ret, ret, L"TestRPMCopyFile: AddRPMDir  failed");

			std::wstring newNXLFile;
			const char* tags = "{\"ip_classification\":[\"secret\"]}";
			ret = pUser->ProtectFile(testFile, tmpDir, tags, newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"TestRPMCopyFile: ProtectFile  failed");
			// Case1: Copy to rpm folder , a unprotected file  exists
			wstring nxlFile = testFile + L".nxl";
			ret = pInst->RPMCopyFile(newNXLFile, nxlFile, false);
			ASSERT_RMXERROR(!ret, ret, L"TestRPMCopyFile: RPMCopyFile-1 failed");
			Assert::IsTrue(fs::exists(nxlFile), L"TestRPMCopyFile: RPMCopyFile-1 failed, copy file not found");
			
			// Case2: Copy to rpm folder, same nxl file exists
			ret = pInst->RPMCopyFile(newNXLFile, nxlFile, false);
			ASSERT_RMXERROR(!ret, ret, L"TestRPMCopyFile: RPMCopyFile-2 failed");
			Assert::IsTrue(fs::exists(nxlFile), L"TestRPMCopyFile: RPMCopyFile-2 failed, copy file not found");

			// Case 3: Copy to non rpm folder, a unprotected file exists
			const wstring& nonRPMDir = m_rmxSess.GenerateTempDir();
			const wstring& dummyFile2 = m_rmxSess.CreaeteDummyTxt(nonRPMDir);
			nxlFile = nonRPMDir + L"dummy.txt.nxl";
			ret = pInst->RPMCopyFile(newNXLFile, nxlFile, false);
			ASSERT_RMXERROR(!ret, ret, L"TestRPMCopyFile: RPMCopyFile-3 failed");
			Assert::IsTrue(fs::exists(nxlFile), L"TestRPMCopyFile: RPMCopyFile-3 failed, copy file not found");

			// Case 4: Copy to non rpm folder, same nxl file exists
			ret = pInst->RPMCopyFile(newNXLFile, nxlFile, false);
			ASSERT_RMXERROR(!ret, ret, L"TestRPMCopyFile: RPMCopyFile-4 failed");
			Assert::IsTrue(fs::exists(nxlFile), L"TestRPMCopyFile: RPMCopyFile-4 failed, copy file not found");

			//// case 5: Copy from non rpm folder - Don't support. Expect to return error
			// since 2020.08: source file in non rpm folder not supported
			//wstring nxlFile2 = nonRPMDir + L"dummy5.txt.nxl";
			//ret = pInst->RPMCopyFile(nxlFile, nxlFile2, false);
			//ASSERT_RMXERROR(ret, ret, L"TestRPMCopyFile: RPMCopyFile-5 failed. Source file in non rpm folder Not supported");
			//Assert::IsTrue(!fs::exists(nxlFile2), L"TestRPMCopyFile: RPMCopyFile-5 failed, copy file should not allowed");

			// case 6: "/" path separator
			wstring newFile6 = nonRPMDir + L"dummy6.txt.nxl";
			wstring testFile6(testFile);
			std::replace(testFile6.begin(), testFile6.end(), L'\\', L'/');
			std::replace(newFile6.begin(), newFile6.end(), L'\\', L'/');
			ret = pInst->RPMCopyFile(testFile6, newFile6, true);
			ASSERT_RMXERROR(!ret, ret, L"TestRPMCopyFile: RPMCopyFile-6 failed. / path separator not working");
			Assert::IsTrue(fs::exists(newFile6), L"TestRPMCopyFile: RPMCopyFile-6 failed, copy file not found");
			Assert::IsTrue(!fs::exists(testFile6), L"TestRPMCopyFile: RPMCopyFile-1 failed, source file not deleted");

			// Case 7: Invalid target path
			wstring invaidPath = nonRPMDir;
			ret = pInst->RPMCopyFile(testFile, invaidPath, true);
			ASSERT_RMXERROR(ret, ret, L"TestRPMCopyFile: RPMCopyFile-7 failed. Invalid target path not allowed");
		}

		TEST_METHOD(TestRPMDeleteFile)
		{
			UTEST_LOG_ENTER;

			m_rmxSess.RunRMXSession();
			IRMXUser* pUser = m_rmxSess.User();
			IRMXInstance* pInst = m_rmxSess.Inst();

			// Prepare nxl file in rpm dir
			const wstring& tmpDir = m_rmxSess.GenerateTempDir();
			const wstring& testFile = m_rmxSess.CreaeteDummyTxt(tmpDir);
			auto ret = pInst->AddRPMDir(tmpDir);
			ASSERT_RMXERROR(!ret, ret, L"TestRPMDeleteFile: AddRPMDir  failed");

			std::wstring newNXLFile;
			const char* tags = "{\"ip_classification\":[\"secret\"]}";
			ret = pUser->ProtectFile(testFile, tmpDir, tags, newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"TestRPMDeleteFile: ProtectFile  failed");

			
			ret = pInst->RPMDeleteFile(newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"TestRPMDeleteFile: RPMDeleteFile  failed");

			Assert::IsTrue(!fs::exists(newNXLFile), L"TestRPMDeleteFile file not deleted");
		}

		TEST_METHOD(TestGetFileRights)
		{
			UTEST_LOG_ENTER;

			m_rmxSess.RunRMXSession();
			IRMXUser* pUser = m_rmxSess.User();
			IRMXInstance* pInst = m_rmxSess.Inst();

			// Prepare nxl file in rpm dir
			const wstring& tmpDir = m_rmxSess.GenerateTempDir();
			const wstring& testFile = m_rmxSess.CreaeteDummyTxt(tmpDir);
			
			std::wstring newNXLFile;
			const char* tags = "{\"ip_classification\":[\"top-secret\"]}";
			auto ret = pUser->ProtectFile(testFile, tmpDir, tags, newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"TestGetFileRights: ProtectFile  failed");

			// Test in non rpm folder
			std::vector<RMXFileRight> rights;
			ret = pInst->GetFileRights(newNXLFile, rights);
			ASSERT_RMXERROR(!ret && rights.size() != 1 && rights.front() != RMX_RIGHT_VIEW, ret, L"TestGetFileRights: GetFileRights-1  failed");

			ret = pInst->AddRPMDir(tmpDir);
			ASSERT_RMXERROR(!ret, ret, L"TestGetFileRights: AddRPMDir  failed");
			rights.clear();
			ret = pInst->GetFileRights(newNXLFile, rights);
			ASSERT_RMXERROR(!ret && rights.size() != 1 && rights.front() != RMX_RIGHT_VIEW, ret, L"TestGetFileRights: GetFileRights-2  failed");

		}

		TEST_METHOD(TestCheckFileRights)
		{
			UTEST_LOG_ENTER;

			m_rmxSess.RunRMXSession();
			IRMXUser* pUser = m_rmxSess.User();
			IRMXInstance* pInst = m_rmxSess.Inst();

			// Prepare nxl file in rpm dir
			const wstring& tmpDir = m_rmxSess.GenerateTempDir();
			const wstring& testFile = m_rmxSess.CreaeteDummyTxt(tmpDir);

			std::wstring newNXLFile;
			const char* tags = "{\"ip_classification\":[\"secret-edit\"]}";
			auto ret = pUser->ProtectFile(testFile, tmpDir, tags, newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"TestGetFileRights: ProtectFile  failed");

			// Test in non rpm folder
			std::vector<RMXFileRight> rights;
			bool allow;
			ret = pInst->CheckFileRight(newNXLFile, RMX_RIGHT_DOWNLOAD, allow);
			ASSERT_RMXERROR(!ret || allow, ret, L"TEstCheckFileRight: CheckFileRight-1  failed");

			ret = pInst->AddRPMDir(tmpDir);
			ASSERT_RMXERROR(!ret, ret, L"TestGetFileRights: AddRPMDir  failed");
			rights.clear();
			ret = pInst->CheckFileRight(newNXLFile, RMX_RIGHT_VIEW, allow);
			ASSERT_RMXERROR(!ret && !allow, ret, L"TEstCheckFileRight: CheckFileRight-2  failed");

		}

		TEST_METHOD(TestGetFileStatus)
		{
			UTEST_LOG_ENTER;

			m_rmxSess.RunRMXSession();
			IRMXUser* pUser = m_rmxSess.User();
			IRMXInstance* pInst = m_rmxSess.Inst();

			// Prepare nxl file in rpm dir
			const wstring& tmpDir = m_rmxSess.GenerateTempDir();
			const wstring& testFile = m_rmxSess.CreaeteDummyTxt(tmpDir);
			
			std::wstring newNXLFile;
			const char* tags = "{\"ip_classification\":[\"top-secret\"]}";
			auto ret = pUser->ProtectFile(testFile, tmpDir, tags, newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"TestGetFileStatus: ProtectFile  failed");

			// Case1: non rpm folder
			bool isProtected = false;
			RMX_RPMFolderRelation dirStatus;
			ret = pInst->GetFileStatus(newNXLFile, isProtected, &dirStatus);
			ASSERT_RMXERROR(!ret || !isProtected || dirStatus != RMX_NonRPMFolder, ret, L"TestGetFileStatus: GetFileStatus-1 failed");

			// Case2: rpm folder
			ret = pInst->AddRPMDir(tmpDir);
			ASSERT_RMXERROR(!ret, ret, L"TestGetFileStatus: AddRPMDir  failed");
			ret = pInst->GetFileStatus(newNXLFile, isProtected, &dirStatus);
			ASSERT_RMXERROR(!ret || !isProtected || dirStatus != RMX_RPMFolder, ret, L"TestGetFileStatus: GetFileStatus-2  failed");

		}

		TEST_METHOD(TestReadFileTags)
		{
			UTEST_LOG_ENTER;

			m_rmxSess.RunRMXSession();
			IRMXUser* pUser = m_rmxSess.User();
			IRMXInstance* pInst = m_rmxSess.Inst();

			// Prepare nxl file in rpm dir
			const wstring& tmpDir = m_rmxSess.GenerateTempDir();
			const wstring& testFile = m_rmxSess.CreaeteDummyTxt(tmpDir);

			std::wstring newNXLFile;
			const char* tags = "{\"ip_classification\":[\"top-secret\"]}";
			auto ret = pUser->ProtectFile(testFile, tmpDir, tags, newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"TestReadFileTags: ProtectFile  failed");

			string outTags;
			ret = pInst->ReadFileTags(newNXLFile, outTags);
			ASSERT_RMXERROR(!ret || outTags != tags, ret, L"TestReadFileTags: ReadFileTags-1 failed");

			ret = pInst->AddRPMDir(tmpDir);
			ASSERT_RMXERROR(!ret, ret, L"TestReadFileTags: AddRPMDir  failed");
			outTags.clear();
			ret = pInst->ReadFileTags(newNXLFile, outTags);
			ASSERT_RMXERROR(!ret || outTags != tags, ret, L"TestGetFileStatus: ReadFileTags-2  failed");

		}

		TEST_METHOD(TestSetViewOverlay)
		{
			UTEST_LOG_ENTER;
			m_rmxSess.RunRMXSession();
			IRMXInstance* pInst = m_rmxSess.Inst();

			RMX_VIEWOVERLAY_PARAMS params;
			
			//
			// Fill display offset to exclude the title bar and status bar.
			params.displayOffsets[0] =10;
			params.displayOffsets[1] = 50;
			params.displayOffsets[2] = 50;
			params.displayOffsets[3] = 10;

			params.vecTags.push_back("{\"ip_classification\":[\"secret\"]}");
			RMX_EVAL_ATTRIBUATE attr;
			attr.key = L"filename";
			attr.value = L"dummy.txt";
			params.vecCtxAttrs.push_back(attr);

			STARTUPINFO si;
			si.dwFlags = STARTF_USESHOWWINDOW;
			si.wShowWindow = FALSE;
			PROCESS_INFORMATION pi;
			wstring appPath = L"C:\\Windows\\System32\\notepad.exe";

			wstring filePath = m_rmxSess.CreaeteDummyTxt(m_rmxSess.GenerateTempDir());

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
				Assert::IsFalse(ResumeThread(pi.hThread) == -1, L"Failed to resume thread");
				//TerminateProcess(pi.hProcess, 0);

				const DWORD result = WaitForSingleObject(pi.hProcess, 50);
				if (result == WAIT_OBJECT_0) {
					// Success
					
				}
				else {
					// Timed out or an error occurred
				}
				params.hTargetWnd = m_rmxSess.FindTopWindow(pi.dwProcessId);			
				
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				Sleep(1000);
				auto ret = pInst->SetViewOveraly(params);
				Sleep(5000);
				ASSERT_RMXERROR(!ret, ret, L"TestSetViewOverlay: SetViewOveraly failed");
				ret = pInst->ClearViewOverlay(params.hTargetWnd);
				ASSERT_RMXERROR(!ret, ret, L"TestSetViewOverlay: SetViewOveraly failed");
			}	
		}

		TEST_METHOD(TestRPMGetFileInfo)
		{
			UTEST_LOG_ENTER;

			m_rmxSess.RunRMXSession();
			IRMXUser* pUser = m_rmxSess.User();
			IRMXInstance* pInst = m_rmxSess.Inst();

			// Prepare nxl file in rpm dir
			const wstring& tmpDir = m_rmxSess.GenerateTempDir();
			const wstring& testFile = m_rmxSess.CreaeteDummyTxt(tmpDir);

			std::wstring newNXLFile;
			const char* tags = "{\"ip_classification\":[\"top-secret\"]}";
			auto ret = pUser->ProtectFile(testFile, tmpDir, tags, newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"TestGetFileStatus: ProtectFile  failed");

			// Case1: non rpm folder
			
			RMX_RPMFolderRelation dirStatus;
			string duid;
			string tagsRet;
			string tokenGroup;
			string owner;
			string info;
			DWORD atrrs;
			BOOL isNXLFile;
			ret = pInst->RPMGetFileInfo(newNXLFile, duid, tagsRet, tokenGroup, owner, info, atrrs, dirStatus, isNXLFile);
			ASSERT_RMXERROR(!ret || !isNXLFile || dirStatus != RMX_NonRPMFolder, ret, L"TestRPMGetFileInfo: RPMGetFileInfo-1 failed");

			// Case2: rpm folder
			ret = pInst->AddRPMDir(tmpDir);
			ASSERT_RMXERROR(!ret, ret, L"TestGetFileStatus: AddRPMDir  failed");
			ret = pInst->RPMGetFileInfo(newNXLFile, duid, tagsRet, tokenGroup, owner, info, atrrs, dirStatus, isNXLFile);
			ASSERT_RMXERROR(!ret || !isNXLFile || dirStatus != RMX_RPMFolder, ret, L"TestRPMGetFileInfo: RPMGetFileInfo-2  failed");

		}

		TEST_METHOD(TestRPMSetFileAttributes)
		{
			UTEST_LOG_ENTER;

			m_rmxSess.RunRMXSession();
			IRMXUser* pUser = m_rmxSess.User();
			IRMXInstance* pInst = m_rmxSess.Inst();

			// Prepare nxl file in rpm dir
			const wstring& tmpDir = m_rmxSess.GenerateTempDir();
			const wstring& testFile = m_rmxSess.CreaeteDummyTxt(tmpDir);

			std::wstring newNXLFile;
			const char* tags = "{\"ip_classification\":[\"top-secret\"]}";
			auto ret = pUser->ProtectFile(testFile, tmpDir, tags, newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"TestGetFileStatus: ProtectFile  failed");

			auto getFileAttrs = [&]() -> DWORD {
				RMX_RPMFolderRelation dirStatus;
				string duid;
				string tagsRet;
				string tokenGroup;
				string owner;
				string info;
				DWORD atrrs;
				BOOL isNXLFile;
				ret = pInst->RPMGetFileInfo(newNXLFile, duid, tagsRet, tokenGroup, owner, info, atrrs, dirStatus, isNXLFile);
				ASSERT_RMXERROR(!ret || !isNXLFile || dirStatus != RMX_NonRPMFolder, ret, L"TestRPMSetFileAttributes: RPMGetFileInfo-1 failed");
				return atrrs;
			};

			// Case1: non rpm folder
			DWORD attrs = getFileAttrs(); 
			attrs |= FILE_ATTRIBUTE_READONLY;
			ret = pInst->RPMSetFileAttributes(newNXLFile, attrs);
			ASSERT_RMXERROR(!ret, ret, L"TestRPMSetFileAttributes: RPMSetFileAttributes-1 failed");
			attrs = getFileAttrs();
			Assert::IsTrue((attrs & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY, L"RPMSetFileAttributes-2 failed");
			
			// Case2: rpm folder
			ret = pInst->AddRPMDir(tmpDir);
			attrs &= ~FILE_ATTRIBUTE_READONLY;
			ret = pInst->RPMSetFileAttributes(newNXLFile, attrs);
			ASSERT_RMXERROR(!ret, ret, L"TestRPMSetFileAttributes: RPMSetFileAttributes-3 failed");
			Assert::IsFalse((attrs & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY, L"RPMSetFileAttributes-4 failed");
		}

	};
}
