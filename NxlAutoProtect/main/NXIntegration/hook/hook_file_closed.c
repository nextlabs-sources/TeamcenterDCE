#include <stdafx.h>
#include <hook/hook_defs.h>
#include <NxlUtils/Windows/ntdll.h>
#include <hook/windows/windows_defs.h>
#include <hook/hook_file_closed.h>
#include <utils_string.h>

#define DVLOG(fmt, ...) {char sMsgBuf[1024];sprintf(sMsgBuf, PROJECT_NAME "!DEBUG!" __FUNCTION__ "():" fmt "\n", ##__VA_ARGS__);OutputDebugStringA(sMsgBuf);}

//
// NtQueryObject 
//
static BOOL is_file_handle(HANDLE handle)
{
	BOOL isFile = FALSE;
	ULONG expectedSize = 0x100;//sizeof(PUBLIC_OBJECT_TYPE_INFORMATION);
	ULONG size = 0;
	NTSTATUS nt = STATUS_INFO_LENGTH_MISMATCH;
	PPUBLIC_OBJECT_TYPE_INFORMATION typeInfo = NULL;
	while(nt == STATUS_INFO_LENGTH_MISMATCH
		&& expectedSize > size)
	{
		auto newblock =  (PPUBLIC_OBJECT_TYPE_INFORMATION)realloc(typeInfo, expectedSize);
		if (newblock == NULL)
		{
			DVLOG("FATAL-ERROR:realloc failed!(original size-%lu new size-%lu)", size, expectedSize);
			break;
		}
		typeInfo = newblock;
		memset(typeInfo, 0, expectedSize);
		size = expectedSize;
		expectedSize = 0;
		//get basic info
		if(NT_SUCCESS(nt = NtQueryObject_inner(handle, ObjectTypeInformation, typeInfo, size, &expectedSize)))
		{
			//DVLOG("[%#x]TypeName='%S' Len=%hu MaxLen=%hu", handle, typeInfo->TypeName.Buffer, typeInfo->TypeName.Length, typeInfo->TypeName.MaximumLength);
			isFile = _wcsicmp(typeInfo->TypeName.Buffer, L"File") == 0;
			goto CLEANUP;
		}
		DVLOG("[%p]NtQueryObject(ObjectTypeInformation, size=%lu) return %#X(ExpectedSize=%lu)", handle, size, nt,  expectedSize);
	}
CLEANUP:
	if(typeInfo != NULL)
		free(typeInfo);
	return isFile;
}
static BOOL is_write_handle(HANDLE handle, BOOL *isWrite)
{
	BOOL ret = FALSE;
	ULONG expectedSize = sizeof(PUBLIC_OBJECT_BASIC_INFORMATION);//somehow ObjectBasicInformation can work only with this size
	ULONG size = 0;
	NTSTATUS nt = STATUS_INFO_LENGTH_MISMATCH;
	PPUBLIC_OBJECT_BASIC_INFORMATION basicInfo = NULL;
	
	while(nt == STATUS_INFO_LENGTH_MISMATCH
		&& expectedSize > size)
	{
		size = expectedSize;
		expectedSize = 0;
		basicInfo =  (PPUBLIC_OBJECT_BASIC_INFORMATION)realloc(basicInfo, size);
		memset(basicInfo, 0, size);
		//get basic info
		if(NT_SUCCESS(nt = NtQueryObject_inner(handle, ObjectBasicInformation, basicInfo, size, &expectedSize)))
		{
			ACCESS_MASK mask = basicInfo->GrantedAccess;
			BOOL fwd = mask & FILE_WRITE_DATA;
			//DVLOG("[%p]==>Attributes:%lu(%#X)", handle, basicInfo->Attributes, basicInfo->Attributes);
			//DVLOG("[%p]GrantedAccess:%#010X(FILE_WRITE_DATA=%d)", handle, mask, fwd);
			//DVLOG("[%p]==>HandleCount:%lu(%#X)", handle, basicInfo->HandleCount, basicInfo->HandleCount);
			//DVLOG("[%p]==>PointerCount:%lu(%#X)", handle, basicInfo->PointerCount, basicInfo->PointerCount);
			*isWrite = fwd;
			ret = TRUE;
			break;
		}
		DVLOG("[%p]NtQueryObject(ObjectBasicInformation, size=%lu) return %#X(ExpectedSize=%lu)", handle, size, nt,  expectedSize);
	}
	free(basicInfo);
	return ret;
}

static std::vector<pfFileClosedHandler> file_closed_handlers;

static pfCloseHandle CloseHandle_original;
static pfCloseHandle CloseHandle_next;
static BOOL closedHandle_hooked = FALSE;

static BOOL CloseHandle_hooked(HANDLE hnd)
{
	BOOL ret;
	char buf[MAX_PATH] = {0};
	BOOL isWrite = FALSE;
	LONGLONG fileSize = 0;
	if(hnd == NULL
		|| !closedHandle_hooked
		|| IsHookInUse((PVOID*)&CloseHandle_next) > 1)
	{
		//skip process when the CloseHandle is called during hooking or inside CloseHandle_hooked
		return CloseHandle_next(hnd);
	}
	if(is_file_handle(hnd))
	{
		//
		if(GetFinalPathNameByHandle(hnd, buf, MAX_PATH, 0 ) > 0)
		{
			if(!string_ends_with(buf, ".log", FALSE))//filter log writing
			{
				LARGE_INTEGER lint;
				//get handle information
				if(is_write_handle(hnd, &isWrite)
					&& isWrite)
				{
					//DVLOG("[%#X]FinishingWriteFile:'%s'", hnd, buf);
				}
				if(GetFileSizeEx(hnd, &lint))
				{
					fileSize = lint.QuadPart;
				}
				else
				{
					DVLOG("[%p]Failed  to obtain file size(%lu)", hnd, GetLastError());
				}
			}
			else
			{
				buf[0] = '\0';
			}
		}
		else
		{
			DWORD errCode = GetLastError();
			if(errCode != ERROR_NOT_SUPPORTED)//filter some known errors
			{
				DVLOG("[%p]==>GetFinalPathNameByHandle Failed(Error:%#x)", hnd, errCode);
			}
		}
	}
	ret = CloseHandle_next(hnd);
	DWORD lasterror = GetLastError();
	//buf name should like \\?\C:\ 
	if(strlen(buf) > 4
		&& file_closed_handlers.size() > 0)
	{
		DVLOG("[%p]File='%s' isWrite=%d fileSize=%lld", hnd, buf, isWrite, fileSize);
		//post close handler
		for (auto ph = file_closed_handlers.begin();ph!= file_closed_handlers.end();ph++)
		{
			//DVLOG("Executing pfWriteFileFinishedHandler('%p', '%s', %d)", handler->handler, buf, isWrite);
			(*ph)(buf + 4, isWrite, fileSize);
		}
	}
	SetLastError(lasterror);
	return ret;
}

//
// Event - WriteFileFinished
//
void register_FileClosedHandler(pfFileClosedHandler handler)
{
	if(handler != NULL)
	{
		if (file_closed_handlers.size() > 0)
		{
			auto h = std::find(file_closed_handlers.begin(), file_closed_handlers.end(), handler);
			if (h != file_closed_handlers.end()) {
				DBGLOG("Duplicated handler!!");
				return;
			}
		}
		DBGLOG("Registering pfWriteFileFinishedHandlerW('%p')", handler);
		file_closed_handlers.push_back(handler);
		if (file_closed_handlers.size() == 1) {
			DBGLOG("HOOKing CloseHandle");
			//HOOK(CloseHandle);//this will install hook on kernel32.dll, which may not work on win10
			HMODULE mKernalbase = GetModuleHandle("kernelbase.dll");
			HOOK_byName(CloseHandle, mKernalbase);
			closedHandle_hooked = TRUE;
		}
	}
}

void unregister_FileClosedHandler(pfFileClosedHandler handler)
{
	if(handler != NULL)
	{
		//search duplicated handler
		if (file_closed_handlers.size() > 0)
		{
			auto h = std::find(file_closed_handlers.begin(), file_closed_handlers.end(), handler);
			if (h != file_closed_handlers.end()) {
				DBGLOG("Found registered handler!!");
				file_closed_handlers.erase(h);
			}
			if (file_closed_handlers.size() == 0) {
				DBGLOG("UNHOOKing CloseHandle");
				UNHOOK(CloseHandle);
				closedHandle_hooked = FALSE;
			}
		}
	}
}
