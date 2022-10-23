/*
	Library:	libugui.dll
*/
#include <hook/hook_defs.h>
#include "libugui.h"

/*
	MB_add_pre_action_callback
	struct CALLBACK_registered_fn_s * __ptr64 __cdecl MB_add_pre_action_callback(void (__cdecl*)(int,void const * __ptr64,void * __ptr64),void * __ptr64)
*/
static pfMB_add_pre_action_callback MB_add_pre_action_callback_original;
CPP_PTR MB_add_pre_action_callback(CALLBACK_PTR p1, void * p2)
{
	CPP_PTR ret = NULL;
	if(MB_add_pre_action_callback_original == NULL)
	{
		auto m = GetModuleHandle("libugui.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libugui.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		MB_add_pre_action_callback_original = (pfMB_add_pre_action_callback)GetProcAddress(m, MB_add_pre_action_callback_FULLNAME);
		if(MB_add_pre_action_callback_original == NULL)
		{
			ERRLOG("GetProcAddress(" MB_add_pre_action_callback_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" MB_add_pre_action_callback_FULLNAME ") returns %p)", MB_add_pre_action_callback_original);
		}
	}
	CALL_NEXT(ret = MB_add_pre_action_callback_original(p1, p2));
	CALL_END("MB_add_pre_action_callback(p1, p2) returns '%p'", ret);
	return ret;
}

/*
	MB_ask_button_name
	char const * __ptr64 __cdecl MB_ask_button_name(int)
*/
static pfMB_ask_button_name MB_ask_button_name_original;
char const * MB_ask_button_name(int p1)
{
	char const * ret = NULL;
	if(MB_ask_button_name_original == NULL)
	{
		auto m = GetModuleHandle("libugui.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libugui.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		MB_ask_button_name_original = (pfMB_ask_button_name)GetProcAddress(m, MB_ask_button_name_FULLNAME);
		if(MB_ask_button_name_original == NULL)
		{
			ERRLOG("GetProcAddress(" MB_ask_button_name_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" MB_ask_button_name_FULLNAME ") returns %p)", MB_ask_button_name_original);
		}
	}
	CALL_NEXT(ret = MB_ask_button_name_original(p1));
	CALL_END("MB_ask_button_name(p1='%d') returns '%s'", p1, ret);
	return ret;
}


