#pragma once

#include <wchar.h>
#include <set>
#include <string>
#include <fstream>
#include <iostream>
#include <experimental/filesystem>
#include <utility>

using namespace std;
namespace fs = std::experimental::filesystem;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#if !defined(__STRINGIZE)
#define __STRINGIZE(X) #X
#endif
#if !defined(_STRINGIZE)
#define _STRINGIZE(X) __STRINGIZE(X)
#endif

#if !defined(_W)
#define __WTEXT(x) L ## x
#define _W(x) __WTEXT(x)
#endif

#define _P(path) std::replace(path.begin(), path.end(), '\\', '\\\\');

#define UTEST_LOG_ENTER Logger::WriteMessage("TEST_METHOD: " __FUNCTION__)
#define UTEST_LOG(msg) Logger::WriteMessage(msg)
#define UTEST_LOG_FMT(fmt, ...) \
	{ \
		wchar_t sMsgBuf[1024]; \
		swprintf_s(sMsgBuf, 1024, fmt, __VA_ARGS__); \
		Logger::WriteMessage(sMsgBuf);\
	}

#define ASSERT_RMXERROR(eval, result, msg) \
	{ \
		if (eval) { \
			UTEST_LOG_FMT(L"%s ({error: %d, %s})", msg, result.GetErrCode(), result.GetErrMessage().c_str());\
			Assert::IsTrue(false, msg); \
		} \
	}


namespace RMXLTest
{
	class RMXLTestSession
	{
	private:
		set<wstring> m_testWorkDirs;
		IRMXInstance* m_pInst;
		IRMXUser* m_pUser;
		bool m_running;
	public:
		IRMXInstance* Inst() const { return m_pInst; }
		IRMXUser* User() const { return m_pUser; }
		RMXLTestSession() : m_pInst(nullptr), m_pUser(nullptr), m_running(false) {}
		~RMXLTestSession()
		{
			TerminateRMXSession();
		}

		wstring GenerateTempDir()
		{
			std::wstring currTime;
			std::time_t tNow = std::time(nullptr);
			wchar_t wstr[20];
			if (std::wcsftime(wstr, 20, L"%Y%m%d%H%M%S", std::localtime(&tNow))) {
				currTime.assign(wstr);
			}

			wstring newTempDir = fs::temp_directory_path().wstring() + L"CADRMXLibTest_" + currTime + L"\\";
			m_testWorkDirs.insert(newTempDir);
			return newTempDir;
		}

		wstring CreaeteDummyTxt(const wstring& dir)
		{
			fs::create_directories(dir);
			wstring file(dir);
			if (dir.find_last_of(L"/\\") != dir.length() - 1)
				file += L"\\";
			file += L"dummy.txt";
			std::ofstream outfile(file);

			outfile << "Hello, I am NextLabs" << std::endl;

			outfile.close();

			return file;
		}

		void InitRMXSession()
		{
			TerminateRMXSession();

			auto ret = RMX_GetCurrentLoggedInUser(m_pInst, m_pUser);
			ASSERT_RMXERROR(!ret, ret, L"RMX_GetCurrentLoggedInUser failed");
			bool finished;
			ret = m_pInst->IsInitFinished(finished);
			ASSERT_RMXERROR(!ret, ret, L"IsInitFinished failed");
			ASSERT_RMXERROR(!finished, ret, L"Init not finished");
		}

		IRMXInstance* RunRMXSession()
		{
			InitRMXSession();
			RMXResult result;
			wchar_t szProcessName[MAX_PATH];
			GetModuleFileName(NULL, &szProcessName[0], MAX_PATH);
			//
			// Register caller app as trusted app
			//
			bool registered;
			if (m_pInst->IsAppRegistered(szProcessName, registered) == 0 && !registered) {
				result = m_pInst->RegisterApp(szProcessName);
				ASSERT_RMXERROR(!result, result, L"RegisterApp failed");
			}
			//
			// Notify RPM driver the RMX gets started to run
			//
			result = m_pInst->NotifyRMXStatus(true);
			ASSERT_RMXERROR(!result, result, L"NotifyRMXStatus failed");

			m_running = true;
			return m_pInst;
		}

		void TerminateRMXSession()
		{
			if (m_pInst == nullptr)
				return;
			
			// Clear up rpm folders
			for (const auto dir : m_testWorkDirs)
			{
				bool isRPMDir = false;
				if (m_pInst->IsRPMFolder(dir.c_str(), isRPMDir) && isRPMDir)
				{
					m_pInst->RemoveRPMDir(dir.c_str(), true);
				}
				try
				{
					error_code ec;
					if (fs::remove_all(dir, ec) == static_cast<std::uintmax_t>(-1))
						Logger::WriteMessage("Failed to remove test folder");
				}
				catch (...)
				{
					Logger::WriteMessage("Exception to remove test folder");
				}
			}
			m_testWorkDirs.clear();

			if (m_running)
			{
				m_pInst->NotifyRMXStatus(false);
				// Unregister
				wchar_t szProcessName[MAX_PATH];
				Assert::IsFalse(GetModuleFileName(NULL, &szProcessName[0], MAX_PATH) == 0);
				UTEST_LOG(szProcessName);
				m_pInst->UnregisterApp(szProcessName);

				m_running = false;
			}

			// Release memory
			RMX_DeleteRMXInstance(m_pInst);
			m_pInst = nullptr;
		}

		HWND FindTopWindow(DWORD pid)
		{
			std::pair<HWND, DWORD> params = { 0, pid };

			// Enumerate the windows using a lambda to process each window
			BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
			{
				auto pParams = (std::pair<HWND, DWORD>*)(lParam);

				DWORD processId;
				if (GetWindowThreadProcessId(hwnd, &processId) && processId == pParams->second)
				{
					// Stop enumerating
					SetLastError(-1);
					pParams->first = hwnd;
					return FALSE;
				}

				// Continue enumerating
				return TRUE;
			}, (LPARAM)&params);

			if (!bResult && GetLastError() == -1 && params.first)
			{
				return params.first;
			}

			return 0;
		}
	};
}