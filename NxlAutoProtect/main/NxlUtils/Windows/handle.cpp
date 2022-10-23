#include <NxlUtils/Windows/ntdll.h>
#include <NxlUtils/Windows/handle.h>

#define DVLOG(fmt, ...) {char sMsgBuf[1024];sprintf(sMsgBuf, PROJECT_NAME "!DEBUG!" __FUNCTION__ "():" fmt "\n", ##__VA_ARGS__);OutputDebugStringA(sMsgBuf);}

DWORD GetGrantAccessFromHandle(HANDLE handle, DWORD *grantAccess)
{
	DWORD ret = ERROR_SUCCESS;
	ULONG expectedSize = sizeof(PUBLIC_OBJECT_BASIC_INFORMATION);//somehow ObjectBasicInformation can work only with this size
	ULONG size = 0;
	PPUBLIC_OBJECT_BASIC_INFORMATION basicInfo = NULL;

	while (expectedSize > size)
	{
		auto newblock = realloc(basicInfo, expectedSize);
		if (newblock == NULL)
		{
			DVLOG("FATAL-ERROR:failed to reallocate %lu bytes for basicInfo(original size=%lu)", expectedSize, size);
			break;
		}
		memset(newblock, 0, expectedSize);
		basicInfo = (PPUBLIC_OBJECT_BASIC_INFORMATION)newblock;
		size = expectedSize;
		expectedSize = 0;
		NTSTATUS nt;
		//get basic info
		if (NT_SUCCESS(nt = NtQueryObject_inner(handle, ObjectBasicInformation, basicInfo, size, &expectedSize)))
		{
			*grantAccess = basicInfo->GrantedAccess;
			break;
		}
		else if (nt != STATUS_INFO_LENGTH_MISMATCH)
		{
			ret = ERROR_INVALID_HANDLE;
			DVLOG("[%p]NtQueryObject(ObjectBasicInformation, size=%lu) return %#X(ExpectedSize=%lu)", handle, size, nt, expectedSize);
			break;
		}
	}
	if(basicInfo != NULL)
		free(basicInfo);
	return ret;
}
// converts
// "\Device\HarddiskVolume3"                                -> "E:"
// "\Device\HarddiskVolume3\Temp"                           -> "E:\Temp"
// "\Device\HarddiskVolume3\Temp\transparent.jpeg"          -> "E:\Temp\transparent.jpeg"
// "\Device\Harddisk1\DP(1)0-0+6\foto.jpg"                  -> "I:\foto.jpg"
// "\Device\TrueCryptVolumeP\Data\Passwords.txt"            -> "P:\Data\Passwords.txt"
// "\Device\Floppy0\Autoexec.bat"                           -> "A:\Autoexec.bat"
// "\Device\CdRom1\VIDEO_TS\VTS_01_0.VOB"                   -> "H:\VIDEO_TS\VTS_01_0.VOB"
// "\Device\Serial1"                                        -> "COM1"
// "\Device\USBSER000"                                      -> "COM4"
// "\Device\Mup\ComputerName\C$\Boot.ini"                   -> "\\ComputerName\C$\Boot.ini"
// "\Device\LanmanRedirector\ComputerName\C$\Boot.ini"      -> "\\ComputerName\C$\Boot.ini"
// "\Device\LanmanRedirector\ComputerName\Shares\Dance.m3u" -> "\\ComputerName\Shares\Dance.m3u"
// returns an error for any other device type
DWORD GetDosPathFromNtPath(const std::wstring &ntPath, std::wstring &dosPath)
{
	DWORD u32_Error;

	if (_wcsnicmp(ntPath.c_str(), L"\\Device\\Serial", 14) == 0 || // e.g. "Serial1"
		_wcsnicmp(ntPath.c_str(), L"\\Device\\UsbSer", 14) == 0)   // e.g. "USBSER000"
	{
		HKEY h_Key;
		if (u32_Error = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Hardware\\DeviceMap\\SerialComm", 0, KEY_QUERY_VALUE, &h_Key))
			return u32_Error;

		WCHAR u16_ComPort[50];

		DWORD u32_Type;
		DWORD u32_Size = sizeof(u16_ComPort);
		if (u32_Error = RegQueryValueExW(h_Key, ntPath.c_str(), 0, &u32_Type, (BYTE*)u16_ComPort, &u32_Size))
		{
			RegCloseKey(h_Key);
			return ERROR_UNKNOWN_PORT;
		}
		RegCloseKey(h_Key);
		dosPath = u16_ComPort;
		return 0;
	}

	if (_wcsnicmp(ntPath.c_str(), L"\\Device\\LanmanRedirector\\", 25) == 0) // Win XP
	{
		dosPath = L"\\\\";
		dosPath += ntPath.c_str() + 25;
		return 0;
	}

	if (_wcsnicmp(ntPath.c_str(), L"\\Device\\Mup\\", 12) == 0) // Win 7
	{
		dosPath = L"\\\\";
		dosPath += ntPath.c_str() + 12;
		return 0;
	}

	WCHAR u16_Drives[300];
	if (!GetLogicalDriveStringsW(300, u16_Drives))
		return GetLastError();

	WCHAR* u16_Drv = u16_Drives;
	while (u16_Drv[0])
	{
		WCHAR* u16_Next = u16_Drv + wcslen(u16_Drv) + 1;

		u16_Drv[2] = 0; // the backslash is not allowed for QueryDosDevice()

		WCHAR u16_NtVolume[1000];
		u16_NtVolume[0] = 0;

		// may return multiple strings!
		// returns very weird strings for network shares
		if (!QueryDosDeviceW(u16_Drv, u16_NtVolume, sizeof(u16_NtVolume) / 2))
			return GetLastError();

		int s32_Len = (int)wcslen(u16_NtVolume);
		if (s32_Len > 0 && _wcsnicmp(ntPath.c_str(), u16_NtVolume, s32_Len) == 0)
		{
			dosPath = u16_Drv;
			dosPath += ntPath.c_str() + s32_Len;
			return 0;
		}

		u16_Drv = u16_Next;
	}
	return ERROR_BAD_PATHNAME;
}
// returns
// "\Device\HarddiskVolume3"                                (Harddisk Drive)
// "\Device\HarddiskVolume3\Temp"                           (Harddisk Directory)
// "\Device\HarddiskVolume3\Temp\transparent.jpeg"          (Harddisk File)
// "\Device\Harddisk1\DP(1)0-0+6\foto.jpg"                  (USB stick)
// "\Device\TrueCryptVolumeP\Data\Passwords.txt"            (Truecrypt Volume)
// "\Device\Floppy0\Autoexec.bat"                           (Floppy disk)
// "\Device\CdRom1\VIDEO_TS\VTS_01_0.VOB"                   (DVD drive)
// "\Device\Serial1"                                        (real COM port)
// "\Device\USBSER000"                                      (virtual COM port)
// "\Device\Mup\ComputerName\C$\Boot.ini"                   (network drive share,  Windows 7)
// "\Device\LanmanRedirector\ComputerName\C$\Boot.ini"      (network drive share,  Windwos XP)
// "\Device\LanmanRedirector\ComputerName\Shares\Dance.m3u" (network folder share, Windwos XP)
// "\Device\Afd"                                            (internet socket)
// "\Device\Console000F"                                    (unique name for any Console handle)
// "\Device\NamedPipe\Pipename"                             (named pipe)
// "\BaseNamedObjects\Objectname"                           (named mutex, named event, named semaphore)
// "\REGISTRY\MACHINE\SOFTWARE\Classes\.txt"                (HKEY_CLASSES_ROOT\.txt)
DWORD GetNtPathFromHandle(HANDLE handle, std::wstring &ntPath)
{
	WCHAR buf[MAX_PATH] = L"";
	if (handle == 0 || handle == INVALID_HANDLE_VALUE)
		return ERROR_INVALID_HANDLE;

	// NtQueryObject() returns STATUS_INVALID_HANDLE for Console handles
	if (IsConsoleHandle(handle))
	{
		swprintf_s(buf, MAX_PATH, L"\\Device\\Console%04X", (DWORD)(DWORD_PTR)handle);
		ntPath = buf;
		return 0;
	}

	OBJECT_NAME_INFORMATION* objNameInfoBuffer = NULL;
	ULONG bufferSize = 0;
	ULONG expectedSize = 2000;
	DWORD ret = 0;

	while (bufferSize < expectedSize) {
		auto newblock = realloc(objNameInfoBuffer, expectedSize);
		if (newblock == NULL)
		{
			DVLOG("FATAL-ERROR:failed to reallocate %lu bytes for objNameInfoBuffer(original size=%lu)", expectedSize, bufferSize);
			break;
		}
		memset(newblock, 0, expectedSize);
		objNameInfoBuffer = (OBJECT_NAME_INFORMATION*)newblock;
		bufferSize = expectedSize;
		expectedSize = 0;

		// IMPORTANT: The return value from NtQueryObject is bullshit! (driver bug?)
		// - The function may return STATUS_NOT_SUPPORTED although it has successfully written to the buffer.
		// - The function returns STATUS_SUCCESS although h_File == 0xFFFFFFFF
		if (STATUS_INFO_LENGTH_MISMATCH == NtQueryObject_inner(handle, ObjectNameInformation, objNameInfoBuffer, bufferSize, &expectedSize))
		{
			DVLOG("[%p]bufferSize=%lu ExpectedSize=%lu", handle, bufferSize, expectedSize);
			continue;
		}

		// On error pk_Info->Buffer is NULL
		if (!objNameInfoBuffer->Name.Buffer || !objNameInfoBuffer->Name.Length)
		{
			ret = ERROR_FILE_NOT_FOUND;
			break;
		}

		objNameInfoBuffer->Name.Buffer[objNameInfoBuffer->Name.Length / 2] = 0; // Length in Bytes!
		ntPath = objNameInfoBuffer->Name.Buffer;
		break;
	}
	if(objNameInfoBuffer != NULL)
		free(objNameInfoBuffer);
	return ret;
}

std::vector<HANDLE> GetHandlesFromProcess(DWORD pid)
{
	std::vector<HANDLE> handles;
	NTSTATUS status;
	// Get the list of all handles in the system
	DWORD size = sizeof(SYSTEM_HANDLE_INFORMATION);
	PSYSTEM_HANDLE_INFORMATION pSysHandleInformation = (PSYSTEM_HANDLE_INFORMATION)malloc(size);
	DWORD needed = 0;
	if (pSysHandleInformation == NULL)
	{
		DVLOG("FATAL-ERROR:failed to allocate %lu bytes for pSysHandleInformation", size);
		goto CLEAN;
	}
	while (!NT_SUCCESS(status = NtQuerySystemInformation_inner(SystemHandleInformation, pSysHandleInformation, size, &needed)))
	{
		if (status != STATUS_INFO_LENGTH_MISMATCH
			|| needed == 0)
		{
			DVLOG("==>Failed Status=%ld(%#X) Needed=%lu", status, status, needed);
			goto CLEAN;// some other error
		}
		// The previously supplied buffer wasn't enough.
		auto newsize = needed + 1024;
		auto newBlock = (PSYSTEM_HANDLE_INFORMATION)realloc(pSysHandleInformation, newsize);
		if (newBlock != NULL)
		{
			pSysHandleInformation = newBlock;
			size = newsize;
		}
		else
		{
			DVLOG("FATAL-ERROR:failed to reallocate %lu bytes for objNameInfoBuffer(original size=%lu)", newsize, size);
			goto CLEAN;
		}
	}

	DVLOG("pSysHandleInformation->Count=%lu pid=%lu (bufSize=%lu)", pSysHandleInformation->Count, pid, size);
	// Iterating through the objects
	for (DWORD i = 0; i < pSysHandleInformation->Count; i++)
	{
		DWORD handlePid = pSysHandleInformation->Handles[i].ProcessID;
		HANDLE hnd = (HANDLE)pSysHandleInformation->Handles[i].HandleNumber;
		//filter by process id
		if (handlePid != pid|| hnd == NULL)
			continue;
		//DVLOG("[%d]Process=%lu hnd=%#.3X",i, pSysHandleInformation->Handles[i].ProcessID, pSysHandleInformation->Handles[i].HandleNumber);
		handles.push_back(hnd);
	}
	DVLOG("handles.size()=%llu", handles.size());
CLEAN:
	if (pSysHandleInformation != NULL)
		free(pSysHandleInformation);
	return handles;
}