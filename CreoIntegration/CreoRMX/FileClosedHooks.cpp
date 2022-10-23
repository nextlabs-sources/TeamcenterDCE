#include "FileClosedHooks.h"

#include <Windows.h>
#include <winternl.h>

#include <madCHook.h>

#include "..\common\CreoRMXLogger.h"
#include "OtkXTypes.h"
#include <PathEx.h>

#include "HookDefs.h"
#include "OtkXSessionCache.h"
#include "OtkXUtils.h"
#include "ProtectController.h"

using namespace std;

namespace FileClosedHooks
{
	//
	//	CloseHandle
	//
	typedef BOOL(WINAPI *pfCloseHandle)(HANDLE hnd);

	typedef NTSTATUS(NTAPI *pfNtQueryObject)(HANDLE, ULONG, PVOID, ULONG, PULONG);
	static pfNtQueryObject NtQueryObject_dynamic = NULL;
	NTSTATUS NtQueryObject_inner(
		HANDLE                   Handle,
		OBJECT_INFORMATION_CLASS ObjectInformationClass,
		PVOID                    ObjectInformation,
		ULONG                    ObjectInformationLength,
		PULONG                   ReturnLength)
	{
		//initialize NtQueryObject
		if (NtQueryObject_dynamic == NULL)
		{
			HMODULE ntdllModule = GetModuleHandle(L"ntdll.dll");
			if (ntdllModule == NULL)
			{
				ntdllModule = LoadLibrary(L"ntdll.dll");
			}
			if(ntdllModule != NULL)
			{
				NtQueryObject_dynamic = (pfNtQueryObject)GetProcAddress(ntdllModule, "NtQueryObject");
				LOG_DEBUG_FMT(L"ntdll Module=%p NtQueryObject=%p", ntdllModule, NtQueryObject_dynamic);
			}
		}

		if (NtQueryObject_dynamic != NULL)
			return NtQueryObject_dynamic(Handle, ObjectInformationClass, ObjectInformation, ObjectInformationLength, ReturnLength);
		else
			return STATUS_INVALID_HANDLE;
	}

	//
	// NtQueryObject 
	//
#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)
#endif
	bool isFileHandle(HANDLE handle)
	{
		bool isFile = false;
		ULONG expectedSize = 0x100;//sizeof(PUBLIC_OBJECT_TYPE_INFORMATION);
		ULONG size = 0;
		NTSTATUS nt = STATUS_INFO_LENGTH_MISMATCH;
		PPUBLIC_OBJECT_TYPE_INFORMATION typeInfo = NULL;
		while (nt == STATUS_INFO_LENGTH_MISMATCH
			&& expectedSize > size)
		{
			size = expectedSize;
			expectedSize = 0;
			typeInfo = (PPUBLIC_OBJECT_TYPE_INFORMATION)realloc(typeInfo, size);
			if (typeInfo == NULL)
			{
				return isFile;
			}
			memset(typeInfo, 0, size);
			//get basic info
			if (NT_SUCCESS(nt = NtQueryObject_inner(handle, ObjectTypeInformation, typeInfo, size, &expectedSize)))
			{
				if (wcsicmp(typeInfo->TypeName.Buffer, L"File") == 0)
				{
					//LOG_DEBUG_FMT(L"NtQueryObject[@0x%p]TypeName=File", handle);
					isFile = true;
				}
				goto CLEANUP;
			}
			//LOG_DEBUG_FMT(L"[%#x]NtQueryObject(ObjectTypeInformation, size=%lu) return %#X(ExpectedSize=%lu)", handle, size, nt, expectedSize);
		}
	CLEANUP:
		free(typeInfo);
		return isFile;
	}

	bool isFileClosed(const std::wstring& filePath)
	{
		HANDLE h = ::CreateFileW(filePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE != h) {
			CloseHandle(h);
			return true;
		}
		return false;
	}

	typedef void(*pfHandleOnAfterFileClose)(const CPathEx& filePathEx);
	std::vector<pfHandleOnAfterFileClose> g_cbListOnFileClosed;

	DEFINE_HOOK_CALLBACK(CloseHandle)
	BOOL CloseHandle_hooked(HANDLE handle)
	{
		if (handle == NULL
			|| CloseHandleInHook
			|| IsHookInUse((PVOID*)&CloseHandle_next) > 1
			|| CProtectController::GetInstance().IsInProcess())
			return CloseHandle_next(handle);

		RMX_SCOPE_GUARD_ENTER(&CloseHandleInHook);
	
		wstring filePath;
		if (isFileHandle(handle))
		{
			filePath = RMXUtils::GetFilePathByHandle(handle);
		}

		BOOL ret = CloseHandle_next(handle);
		
		if (!filePath.empty() && isFileClosed(filePath))
		{
			CPathEx pathEx(filePath.c_str());
			for (const auto& pCB : g_cbListOnFileClosed)
			{
				pCB(pathEx);
			}
		}

		return ret;
	}

	void IpemJPGHandler(const CPathEx& pathEx)
	{
		if (wcsicmp(pathEx.GetExtension().c_str(), L".jpg") == 0)
		{
			LOG_DEBUG_FMT(L"IpemJPGHandler: %s", pathEx.c_str());
			ProtectControllerHolder().ProcessFile(pathEx.ToString(), PROTECT_WATCHER_SAVE_TC);
		}
	}

	void Drw2DxfHandler(const CPathEx& pathEx)
	{
		if ((wcsicmp(pathEx.GetExtension().c_str(), L".dxf") == 0) && 
			DXFFILE_BLACKLIST.find(pathEx.GetFileName()) == DXFFILE_BLACKLIST.end())
		{
			auto pCurrMdl = OtkXUtils::OtkX_GetCurrentModel();
			COtkXFile srcMainFile(CXS2W(pCurrMdl->GetOrigin()));
			if (srcMainFile.IsDrw())
			{
				LOG_DEBUG_FMT(L"Drw2DxfHandler: %s", pathEx.c_str());
				ProtectControllerHolder().ProcessFile(pathEx.ToString(), PROTECT_WATCHER_EXPORT);
			}
		}
	}

	void Asm2MotionEnvlpHandler(const CPathEx& pathEx)
	{
		if (!ProtectControllerHolder().IsWatcherRunning(PROTECT_WATCHER_EXPORT_PLAYBACK_MOTIONENVELOPE))
			return;
		
		bool allowProcess = false;
		if ((wcsicmp(pathEx.GetExtension().c_str(), L".stl") == 0) ||
			(wcsicmp(pathEx.GetExtension().c_str(), L".wrl") == 0))
		{
			allowProcess = true;
		}
		else
		{
			OtkXUtils::OtkXFileNameData nameData(pathEx.ToString());
			if (wcsicmp(nameData.GetExtension().c_str(), L".prt") == 0)
			{
				allowProcess = true;
			}
		}

		if (allowProcess)
		{
			LOG_DEBUG_FMT(L"Asm2MotionEnvlpHandler: %s", pathEx.c_str());
			ProtectControllerHolder().ProcessWatchedFiles(PROTECT_WATCHER_EXPORT);
		}
	}

	void ExportShrinkWrapHandler(const CPathEx& pathEx)
	{
		if ((wcsicmp(pathEx.GetExtension().c_str(), L".stl") == 0) ||
			(wcsicmp(pathEx.GetExtension().c_str(), L".wrl") == 0))
		{
			LOG_DEBUG_FMT(L"ExportShrinkWrapHandler: %s", pathEx.c_str());
			ProtectControllerHolder().ProcessWatchedFiles(PROTECT_WATCHER_EXPORT);
		}
	}

	// Since 5.4 In creo 4 M060, CreateFile Hook not working for eps
	void ExportEpsHandler(const CPathEx& pathEx)
	{
		if ((wcsicmp(pathEx.GetExtension().c_str(), L".eps") == 0))
		{
			LOG_DEBUG_FMT(L"ExportEpsHandler: %s", pathEx.c_str());
			ProtectControllerHolder().WatchFile(pathEx.c_str());
		}
	}

	bool hookCloseHandle()
	{
		HOOK_API("KernelBase.dll", CloseHandle);
		return IS_HOOKED(CloseHandle);
	}

	void UnhookCloseHandle()
	{
		UNHOOK(CloseHandle);
	}

	void Install_IpemJPG()
	{
		if (hookCloseHandle())
			g_cbListOnFileClosed.push_back(IpemJPGHandler);
	}

	void Uninstall_IpemJPG()
	{
		UnhookCloseHandle();
	}

	void Install_Export()
	{
		if (hookCloseHandle())
		{
			g_cbListOnFileClosed.push_back(Drw2DxfHandler);
			g_cbListOnFileClosed.push_back(Asm2MotionEnvlpHandler);
			g_cbListOnFileClosed.push_back(ExportShrinkWrapHandler);
			g_cbListOnFileClosed.push_back(ExportEpsHandler);
		}
	}

	void Uninstall_Export()
	{
		UnhookCloseHandle();
		g_cbListOnFileClosed.clear();
	}

} //namespace FileClosedHooks

