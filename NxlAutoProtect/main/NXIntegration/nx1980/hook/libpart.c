/*
	Library:	libpart.dll
*/
#include <hook/hook_defs.h>
#include "libpart.h"

/*
	PartFileStream_GetOSPath
	public: char const * __ptr64 __cdecl UGS::Part::PartFileStream::GetOSPath(void)const __ptr64
*/
static pfPartFileStream_GetOSPath PartFileStream_GetOSPath_original;
char const * PartFileStream_GetOSPath(PartFileStream_PTR obj)
{
	char const * ret = NULL;
	if(PartFileStream_GetOSPath_original == NULL)
	{
		auto m = GetModuleHandle("libpart.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpart.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PartFileStream_GetOSPath_original = (pfPartFileStream_GetOSPath)GetProcAddress(m, PartFileStream_GetOSPath_FULLNAME);
		if(PartFileStream_GetOSPath_original == NULL)
		{
			ERRLOG("GetProcAddress(" PartFileStream_GetOSPath_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PartFileStream_GetOSPath_FULLNAME ") returns %p)", PartFileStream_GetOSPath_original);
		}
	}
	CALL_NEXT(ret = PartFileStream_GetOSPath_original(obj));
	CALL_END("PartFileStream_GetOSPath(obj='%p') returns '%s'", obj, ret);
	return ret;
}

/*
	SaveInput_GetPartTag
	public: virtual unsigned int __cdecl UGS::Part::SaveInput::GetPartTag(void)const __ptr64
*/
static pfSaveInput_GetPartTag SaveInput_GetPartTag_original;
unsigned int SaveInput_GetPartTag(CPP_PTR obj)
{
	unsigned int ret = 0;
	if(SaveInput_GetPartTag_original == NULL)
	{
		auto m = GetModuleHandle("libpart.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpart.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		SaveInput_GetPartTag_original = (pfSaveInput_GetPartTag)GetProcAddress(m, SaveInput_GetPartTag_FULLNAME);
		if(SaveInput_GetPartTag_original == NULL)
		{
			ERRLOG("GetProcAddress(" SaveInput_GetPartTag_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" SaveInput_GetPartTag_FULLNAME ") returns %p)", SaveInput_GetPartTag_original);
		}
	}
	CALL_NEXT(ret = SaveInput_GetPartTag_original(obj));
	CALL_END("SaveInput_GetPartTag(obj='%p') returns '%u'", obj, ret);
	return ret;
}

/*
	PartFileStream_IsForWrite
	public: bool __cdecl UGS::Part::PartFileStream::IsForWrite(void)const __ptr64
*/
static pfPartFileStream_IsForWrite PartFileStream_IsForWrite_original;
BOOL PartFileStream_IsForWrite(PartFileStream_PTR obj)
{
	BOOL ret = FALSE;
	if(PartFileStream_IsForWrite_original == NULL)
	{
		auto m = GetModuleHandle("libpart.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpart.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PartFileStream_IsForWrite_original = (pfPartFileStream_IsForWrite)GetProcAddress(m, PartFileStream_IsForWrite_FULLNAME);
		if(PartFileStream_IsForWrite_original == NULL)
		{
			ERRLOG("GetProcAddress(" PartFileStream_IsForWrite_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PartFileStream_IsForWrite_FULLNAME ") returns %p)", PartFileStream_IsForWrite_original);
		}
	}
	CALL_NEXT(ret = PartFileStream_IsForWrite_original(obj));
	CALL_END("PartFileStream_IsForWrite(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PartFileStream_IsReadOnly
	public: bool __cdecl UGS::Part::PartFileStream::IsReadOnly(void)const __ptr64
*/
static pfPartFileStream_IsReadOnly PartFileStream_IsReadOnly_original;
BOOL PartFileStream_IsReadOnly(PartFileStream_PTR obj)
{
	BOOL ret = FALSE;
	if(PartFileStream_IsReadOnly_original == NULL)
	{
		auto m = GetModuleHandle("libpart.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpart.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		PartFileStream_IsReadOnly_original = (pfPartFileStream_IsReadOnly)GetProcAddress(m, PartFileStream_IsReadOnly_FULLNAME);
		if(PartFileStream_IsReadOnly_original == NULL)
		{
			ERRLOG("GetProcAddress(" PartFileStream_IsReadOnly_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" PartFileStream_IsReadOnly_FULLNAME ") returns %p)", PartFileStream_IsReadOnly_original);
		}
	}
	CALL_NEXT(ret = PartFileStream_IsReadOnly_original(obj));
	CALL_END("PartFileStream_IsReadOnly(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	PartFileStream_SetReadOnly
	public: void __cdecl UGS::Part::PartFileStream::SetReadOnly(void) __ptr64
*/
static pfPartFileStream_SetReadOnly PartFileStream_SetReadOnly_original;
void PartFileStream_SetReadOnly(PartFileStream_PTR obj)
{
	if(PartFileStream_SetReadOnly_original == NULL)
	{
		auto m = GetModuleHandle("libpart.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libpart.dll') Failed(Error=%lu)", GetLastError());
			return;
		}
		PartFileStream_SetReadOnly_original = (pfPartFileStream_SetReadOnly)GetProcAddress(m, PartFileStream_SetReadOnly_FULLNAME);
		if(PartFileStream_SetReadOnly_original == NULL)
		{
			ERRLOG("GetProcAddress(" PartFileStream_SetReadOnly_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return;
		}
		else
		{
			DBGLOG("GetProcAddress(" PartFileStream_SetReadOnly_FULLNAME ") returns %p)", PartFileStream_SetReadOnly_original);
		}
	}
	CALL_NEXT(PartFileStream_SetReadOnly_original(obj));
	CALL_END("PartFileStream_SetReadOnly(obj='%p')", obj);
}


