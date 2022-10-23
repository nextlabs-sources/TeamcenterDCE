#include "stdafx.h"

namespace RMXLTest
{
	TEST_CLASS(RMXAPISampleTest)
	{	
	private:

		RMXLTestSession m_rmxSess;

	public:
		TEST_CLASS_INITIALIZE(RMXAPISampleTest_Class_Init)
		{
			UTEST_LOG_ENTER;		
		}

		TEST_CLASS_CLEANUP(RMXAPISampleTest_Class_Clearup)
		{
			UTEST_LOG_ENTER;
		}

		TEST_METHOD_INITIALIZE(RMXAPISampleTest_Method_Init)
		{
			UTEST_LOG_ENTER;
		}

		TEST_METHOD_CLEANUP(RMXAPISampleTest_Method_Cleanup)
		{
			UTEST_LOG_ENTER;
			m_rmxSess.TerminateRMXSession();
		}

		TEST_METHOD(TestSampe1)
		{
			IRMXInstance* pInst = nullptr;
			IRMXUser* pUser = nullptr;
			
			//
			// Initialize RMX instance with current logged users via RMD
			//
			RMXResult result = RMX_GetCurrentLoggedInUser(pInst, pUser);
			if (!result) {
				std::wcout << result.GetErrMessage() << endl;		
				return;
			}

			wchar_t szProcessName[MAX_PATH];
			GetModuleFileName(NULL, &szProcessName[0], MAX_PATH);
			//
			// Register caller app as trusted app
			//
			bool registered;
			if (pInst->IsAppRegistered(szProcessName, registered) == 0 && !registered) {
				result = pInst->RegisterApp(szProcessName);
				if (!result) {
					std::wcout << result.GetErrMessage() << endl;
					return;
				}
			}
			//
			// Notify RPM driver the RMX gets started to run
			//
			result = pInst->NotifyRMXStatus(true);
			if (!result) {
				std::wcout << result.GetErrMessage() << endl;
				return;
			}

			// ....Do other operations

			//
			// Uninitialize RMX instance when app or plugin terminates
			// 
			RMX_DeleteRMXInstance(pInst);
		}

		TEST_METHOD(TestSampe2)
		{
			m_rmxSess.InitRMXSession();
			IRMXInstance* pInst = m_rmxSess.Inst();
			IRMXUser* pUser = m_rmxSess.User();

			//
			// Check if specified directory is RPM folder
			//
			bool isRPMDir = false;
			const wchar_t* dir = L"c:\\users\\rpmtest";
			RMXResult result = pInst->IsRPMFolder(dir, isRPMDir);
			if (!result) {
				std::wcout << result.GetErrMessage() << endl;
				return;
			}
			
			//
			// Add new RPM folder
			//
			if (!isRPMDir) {
				result = pInst->AddRPMDir(dir, 1);
				if (!result) {
					std::wcout << result.GetErrMessage() << endl;
					return;
				}
			}
		}

		TEST_METHOD(TestSampe3)
		{
			m_rmxSess.InitRMXSession();
			IRMXInstance* pInst = m_rmxSess.Inst();
			IRMXUser* pUser = m_rmxSess.User();

			//
			// Protect file
			//		
			std::wstring newNXLFile;
			const wchar_t* filePath = L"c:\\users\\rpmtest\\helloworld.txt";
			const char* tags = "{\"ip_classification\":[\"secret\"]}";
			RMXResult result = pUser->ProtectFile(filePath, L"c:\\users\\rpmtest", tags, newNXLFile);
			if (!result) {
				std::wcout << result.GetErrMessage() << std::endl;
				return;
			}
		}

		TEST_METHOD(TestSampe4)
		{
			m_rmxSess.InitRMXSession();
			IRMXInstance* pInst = m_rmxSess.Inst();
			IRMXUser* pUser = m_rmxSess.User();

			//
			// Evaluate right for a protected file
			//		
			const wchar_t* filePath = L"c:\\users\\rpmtest\\helloworld.txt.nxl";
			bool allowed = false;
			RMXResult result = pInst->CheckFileRight(filePath, RMX_RIGHT_EDIT, allowed);
			if (!result) {
				std::wcout << result.GetErrMessage() << std::endl;
				return;
			}
		}
	};
}
