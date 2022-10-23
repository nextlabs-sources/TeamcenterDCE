/*
	Library:	libugmr.dll
*/
#include <hook/hook_defs.h>
#include "libugmr.h"

/*
	TableEntry_GetPartTag
	public: unsigned int __cdecl UGS::PDM::TableEntry::GetPartTag(void)const __ptr64
*/
static pfTableEntry_GetPartTag TableEntry_GetPartTag_original;
unsigned int TableEntry_GetPartTag(TableEntry_PTR obj)
{
	unsigned int ret = 0;
	if(TableEntry_GetPartTag_original == NULL)
	{
		auto m = GetModuleHandle("libugmr.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libugmr.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		TableEntry_GetPartTag_original = (pfTableEntry_GetPartTag)GetProcAddress(m, TableEntry_GetPartTag_FULLNAME);
		if(TableEntry_GetPartTag_original == NULL)
		{
			ERRLOG("GetProcAddress(" TableEntry_GetPartTag_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" TableEntry_GetPartTag_FULLNAME ") returns %p)", TableEntry_GetPartTag_original);
		}
	}
	CALL_NEXT(ret = TableEntry_GetPartTag_original(obj));
	CALL_END("TableEntry_GetPartTag(obj='%p') returns '%u'", obj, ret);
	return ret;
}

/*
	TableEntry_GetPathName
	public: char const * __ptr64 __cdecl UGS::PDM::TableEntry::GetPathName(void)const __ptr64
*/
static pfTableEntry_GetPathName TableEntry_GetPathName_original;
char const * TableEntry_GetPathName(TableEntry_PTR obj)
{
	char const * ret = NULL;
	if(TableEntry_GetPathName_original == NULL)
	{
		auto m = GetModuleHandle("libugmr.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libugmr.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		TableEntry_GetPathName_original = (pfTableEntry_GetPathName)GetProcAddress(m, TableEntry_GetPathName_FULLNAME);
		if(TableEntry_GetPathName_original == NULL)
		{
			ERRLOG("GetProcAddress(" TableEntry_GetPathName_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" TableEntry_GetPathName_FULLNAME ") returns %p)", TableEntry_GetPathName_original);
		}
	}
	CALL_NEXT(ret = TableEntry_GetPathName_original(obj));
	CALL_END("TableEntry_GetPathName(obj='%p') returns '%s'", obj, ret);
	return ret;
}

/*
	TableEntry_LookupBySpec
	public: static class UGS::PDM::TableEntry * __ptr64 __cdecl UGS::PDM::TableEntry::LookupBySpec(char const * __ptr64)
*/
static pfTableEntry_LookupBySpec TableEntry_LookupBySpec_original;
TableEntry_PTR TableEntry_LookupBySpec(char const * p1)
{
	TableEntry_PTR ret = NULL;
	if(TableEntry_LookupBySpec_original == NULL)
	{
		auto m = GetModuleHandle("libugmr.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libugmr.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		TableEntry_LookupBySpec_original = (pfTableEntry_LookupBySpec)GetProcAddress(m, TableEntry_LookupBySpec_FULLNAME);
		if(TableEntry_LookupBySpec_original == NULL)
		{
			ERRLOG("GetProcAddress(" TableEntry_LookupBySpec_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" TableEntry_LookupBySpec_FULLNAME ") returns %p)", TableEntry_LookupBySpec_original);
		}
	}
	CALL_NEXT(ret = TableEntry_LookupBySpec_original(p1));
	CALL_END("TableEntry_LookupBySpec(p1='%s') returns '%p'", p1, ret);
	return ret;
}

/*
	TableEntry_PrintToSyslog
	public: void __cdecl UGS::PDM::TableEntry::PrintToSyslog(void)const __ptr64
*/
static pfTableEntry_PrintToSyslog TableEntry_PrintToSyslog_original;
void TableEntry_PrintToSyslog(TableEntry_PTR obj)
{
	if(TableEntry_PrintToSyslog_original == NULL)
	{
		auto m = GetModuleHandle("libugmr.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libugmr.dll') Failed(Error=%lu)", GetLastError());
			return;
		}
		TableEntry_PrintToSyslog_original = (pfTableEntry_PrintToSyslog)GetProcAddress(m, TableEntry_PrintToSyslog_FULLNAME);
		if(TableEntry_PrintToSyslog_original == NULL)
		{
			ERRLOG("GetProcAddress(" TableEntry_PrintToSyslog_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return;
		}
		else
		{
			DBGLOG("GetProcAddress(" TableEntry_PrintToSyslog_FULLNAME ") returns %p)", TableEntry_PrintToSyslog_original);
		}
	}
	CALL_NEXT(TableEntry_PrintToSyslog_original(obj));
	CALL_END("TableEntry_PrintToSyslog(obj='%p')", obj);
}


