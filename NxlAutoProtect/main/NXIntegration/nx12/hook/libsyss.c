/*
	Library:	libsyss.dll
*/
#include <hook/hook_defs.h>
#include "libsyss.h"

/*
	CALLBACK_remove_function
	void __cdecl CALLBACK_remove_function(struct CALLBACK_registered_fn_s * __ptr64)
*/
static pfCALLBACK_remove_function CALLBACK_remove_function_original;
void CALLBACK_remove_function(CPP_PTR p1)
{
	if(CALLBACK_remove_function_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return;
		}
		CALLBACK_remove_function_original = (pfCALLBACK_remove_function)GetProcAddress(m, CALLBACK_remove_function_FULLNAME);
		if(CALLBACK_remove_function_original == NULL)
		{
			ERRLOG("GetProcAddress(" CALLBACK_remove_function_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return;
		}
		else
		{
			DBGLOG("GetProcAddress(" CALLBACK_remove_function_FULLNAME ") returns %p)", CALLBACK_remove_function_original);
		}
	}
	CALL_NEXT(CALLBACK_remove_function_original(p1));
	CALL_END("CALLBACK_remove_function(p1='%p')", p1);
}

/*
	Exception_CanAcknowledge
	public: virtual bool __cdecl UGS::Error::Exception::CanAcknowledge(void)const __ptr64
*/
static pfException_CanAcknowledge Exception_CanAcknowledge_original;
BOOL Exception_CanAcknowledge(UException_PTR obj)
{
	BOOL ret = FALSE;
	if(Exception_CanAcknowledge_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		Exception_CanAcknowledge_original = (pfException_CanAcknowledge)GetProcAddress(m, Exception_CanAcknowledge_FULLNAME);
		if(Exception_CanAcknowledge_original == NULL)
		{
			ERRLOG("GetProcAddress(" Exception_CanAcknowledge_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" Exception_CanAcknowledge_FULLNAME ") returns %p)", Exception_CanAcknowledge_original);
		}
	}
	CALL_NEXT(ret = Exception_CanAcknowledge_original(obj));
	CALL_END("Exception_CanAcknowledge(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	ERROR_ask_fail_message
	char const * __ptr64 __cdecl ERROR_ask_fail_message(int)
*/
static pfERROR_ask_fail_message ERROR_ask_fail_message_original;
char const * ERROR_ask_fail_message(int p1)
{
	char const * ret = NULL;
	if(ERROR_ask_fail_message_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		ERROR_ask_fail_message_original = (pfERROR_ask_fail_message)GetProcAddress(m, ERROR_ask_fail_message_FULLNAME);
		if(ERROR_ask_fail_message_original == NULL)
		{
			ERRLOG("GetProcAddress(" ERROR_ask_fail_message_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" ERROR_ask_fail_message_FULLNAME ") returns %p)", ERROR_ask_fail_message_original);
		}
	}
	CALL_NEXT(ret = ERROR_ask_fail_message_original(p1));
	CALL_END("ERROR_ask_fail_message(p1='%d') returns '%s'", p1, ret);
	return ret;
}

/*
	TEXT_create_string
	struct TEXT_s * __ptr64 __cdecl TEXT_create_string(char const * __ptr64)
*/
static pfTEXT_create_string TEXT_create_string_original;
UTEXT_PTR TEXT_create_string(char const * p1)
{
	UTEXT_PTR ret = NULL;
	if(TEXT_create_string_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		TEXT_create_string_original = (pfTEXT_create_string)GetProcAddress(m, TEXT_create_string_FULLNAME);
		if(TEXT_create_string_original == NULL)
		{
			ERRLOG("GetProcAddress(" TEXT_create_string_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" TEXT_create_string_FULLNAME ") returns %p)", TEXT_create_string_original);
		}
	}
	CALL_NEXT(ret = TEXT_create_string_original(p1));
	CALL_END("TEXT_create_string(p1='%s') returns '%p'", p1, ret);
	return ret;
}

/*
	TEXT_free
	void __cdecl TEXT_free(struct TEXT_s * __ptr64)
*/
static pfTEXT_free TEXT_free_original;
void TEXT_free(UTEXT_PTR p1)
{
	if(TEXT_free_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return;
		}
		TEXT_free_original = (pfTEXT_free)GetProcAddress(m, TEXT_free_FULLNAME);
		if(TEXT_free_original == NULL)
		{
			ERRLOG("GetProcAddress(" TEXT_free_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return;
		}
		else
		{
			DBGLOG("GetProcAddress(" TEXT_free_FULLNAME ") returns %p)", TEXT_free_original);
		}
	}
	CALL_NEXT(TEXT_free_original(p1));
	CALL_END("TEXT_free(p1='%p')", p1);
}

/*
	TEXT_strcpy
	void __cdecl TEXT_strcpy(char * __ptr64,struct TEXT_s const * __ptr64)
*/
static pfTEXT_strcpy TEXT_strcpy_original;
void TEXT_strcpy(char * p1, UTEXT_PTR p2)
{
	if(TEXT_strcpy_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return;
		}
		TEXT_strcpy_original = (pfTEXT_strcpy)GetProcAddress(m, TEXT_strcpy_FULLNAME);
		if(TEXT_strcpy_original == NULL)
		{
			ERRLOG("GetProcAddress(" TEXT_strcpy_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return;
		}
		else
		{
			DBGLOG("GetProcAddress(" TEXT_strcpy_FULLNAME ") returns %p)", TEXT_strcpy_original);
		}
	}
	CALL_NEXT(TEXT_strcpy_original(p1, p2));
	CALL_END("TEXT_strcpy(p1='%s', p2='%p')", p1, p2);
}

/*
	TEXT_strlen
	int __cdecl TEXT_strlen(struct TEXT_s const * __ptr64)
*/
static pfTEXT_strlen TEXT_strlen_original;
int TEXT_strlen(UTEXT_PTR p1)
{
	int ret = 0;
	if(TEXT_strlen_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		TEXT_strlen_original = (pfTEXT_strlen)GetProcAddress(m, TEXT_strlen_FULLNAME);
		if(TEXT_strlen_original == NULL)
		{
			ERRLOG("GetProcAddress(" TEXT_strlen_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" TEXT_strlen_FULLNAME ") returns %p)", TEXT_strlen_original);
		}
	}
	CALL_NEXT(ret = TEXT_strlen_original(p1));
	CALL_END("TEXT_strlen(p1='%p') returns '%d'", p1, ret);
	return ret;
}

/*
	Exception_acknowledge
	public: virtual void __cdecl UGS::Error::Exception::acknowledge(void)const __ptr64
*/
static pfException_acknowledge Exception_acknowledge_original;
void Exception_acknowledge(UException_PTR obj)
{
	if(Exception_acknowledge_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return;
		}
		Exception_acknowledge_original = (pfException_acknowledge)GetProcAddress(m, Exception_acknowledge_FULLNAME);
		if(Exception_acknowledge_original == NULL)
		{
			ERRLOG("GetProcAddress(" Exception_acknowledge_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return;
		}
		else
		{
			DBGLOG("GetProcAddress(" Exception_acknowledge_FULLNAME ") returns %p)", Exception_acknowledge_original);
		}
	}
	CALL_NEXT(Exception_acknowledge_original(obj));
	CALL_END("Exception_acknowledge(obj='%p')", obj);
}

/*
	UString_append_0
	public: class UGS::UString & __ptr64 __cdecl UGS::UString::append(class UGS::UString const & __ptr64) __ptr64
*/
static pfUString_append_0 UString_append_0_original;
UString_REF UString_append_0(UString_PTR obj, UString_REF p1)
{
	UString_REF ret = NULL;
	if(UString_append_0_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		UString_append_0_original = (pfUString_append_0)GetProcAddress(m, UString_append_0_FULLNAME);
		if(UString_append_0_original == NULL)
		{
			ERRLOG("GetProcAddress(" UString_append_0_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" UString_append_0_FULLNAME ") returns %p)", UString_append_0_original);
		}
	}
	CALL_NEXT(ret = UString_append_0_original(obj, p1));
	CALL_END("UString_append_0(UString_utf8_str(obj)='%s', UString_utf8_str(p1)='%s') returns '%s'", UString_utf8_str(obj), UString_utf8_str(p1), UString_utf8_str(ret));
	return ret;
}

/*
	UString_append_1
	public: class UGS::UString & __ptr64 __cdecl UGS::UString::append(char) __ptr64
*/
static pfUString_append_1 UString_append_1_original;
UString_REF UString_append_1(UString_PTR obj, char p1)
{
	UString_REF ret = NULL;
	if(UString_append_1_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		UString_append_1_original = (pfUString_append_1)GetProcAddress(m, UString_append_1_FULLNAME);
		if(UString_append_1_original == NULL)
		{
			ERRLOG("GetProcAddress(" UString_append_1_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" UString_append_1_FULLNAME ") returns %p)", UString_append_1_original);
		}
	}
	CALL_NEXT(ret = UString_append_1_original(obj, p1));
	CALL_END("UString_append_1(UString_utf8_str(obj)='%s', p1='%c') returns '%s'", UString_utf8_str(obj), p1, UString_utf8_str(ret));
	return ret;
}

/*
	UString_append_2
	public: class UGS::UString & __ptr64 __cdecl UGS::UString::append(unsigned short) __ptr64
*/
static pfUString_append_2 UString_append_2_original;
UString_REF UString_append_2(UString_PTR obj, unsigned short p1)
{
	UString_REF ret = NULL;
	if(UString_append_2_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		UString_append_2_original = (pfUString_append_2)GetProcAddress(m, UString_append_2_FULLNAME);
		if(UString_append_2_original == NULL)
		{
			ERRLOG("GetProcAddress(" UString_append_2_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" UString_append_2_FULLNAME ") returns %p)", UString_append_2_original);
		}
	}
	CALL_NEXT(ret = UString_append_2_original(obj, p1));
	CALL_END("UString_append_2(UString_utf8_str(obj)='%s', p1) returns '%s'", UString_utf8_str(obj), UString_utf8_str(ret));
	return ret;
}

/*
	UString_append_3
	public: class UGS::UString & __ptr64 __cdecl UGS::UString::append(char const * __ptr64) __ptr64
*/
static pfUString_append_3 UString_append_3_original;
UString_REF UString_append_3(UString_PTR obj, char const * p1)
{
	UString_REF ret = NULL;
	if(UString_append_3_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		UString_append_3_original = (pfUString_append_3)GetProcAddress(m, UString_append_3_FULLNAME);
		if(UString_append_3_original == NULL)
		{
			ERRLOG("GetProcAddress(" UString_append_3_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" UString_append_3_FULLNAME ") returns %p)", UString_append_3_original);
		}
	}
	CALL_NEXT(ret = UString_append_3_original(obj, p1));
	CALL_END("UString_append_3(UString_utf8_str(obj)='%s', p1='%s') returns '%s'", UString_utf8_str(obj), p1, UString_utf8_str(ret));
	return ret;
}

/*
	UString_append_4
	public: class UGS::UString & __ptr64 __cdecl UGS::UString::append(struct TEXT_s const * __ptr64) __ptr64
*/
static pfUString_append_4 UString_append_4_original;
UString_REF UString_append_4(UString_PTR obj, UTEXT_PTR p1)
{
	UString_REF ret = NULL;
	if(UString_append_4_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		UString_append_4_original = (pfUString_append_4)GetProcAddress(m, UString_append_4_FULLNAME);
		if(UString_append_4_original == NULL)
		{
			ERRLOG("GetProcAddress(" UString_append_4_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" UString_append_4_FULLNAME ") returns %p)", UString_append_4_original);
		}
	}
	CALL_NEXT(ret = UString_append_4_original(obj, p1));
	CALL_END("UString_append_4(UString_utf8_str(obj)='%s', p1='%p') returns '%s'", UString_utf8_str(obj), p1, UString_utf8_str(ret));
	return ret;
}

/*
	Exception_askCode
	public: int __cdecl UGS::Error::Exception::askCode(void)const __ptr64
*/
static pfException_askCode Exception_askCode_original;
int Exception_askCode(UException_PTR obj)
{
	int ret = 0;
	if(Exception_askCode_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		Exception_askCode_original = (pfException_askCode)GetProcAddress(m, Exception_askCode_FULLNAME);
		if(Exception_askCode_original == NULL)
		{
			ERRLOG("GetProcAddress(" Exception_askCode_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" Exception_askCode_FULLNAME ") returns %p)", Exception_askCode_original);
		}
	}
	CALL_NEXT(ret = Exception_askCode_original(obj));
	CALL_END("Exception_askCode(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	Exception_askFileName
	public: char const * __ptr64 __cdecl UGS::Error::Exception::askFileName(void)const __ptr64
*/
static pfException_askFileName Exception_askFileName_original;
char const * Exception_askFileName(UException_PTR obj)
{
	char const * ret = NULL;
	if(Exception_askFileName_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		Exception_askFileName_original = (pfException_askFileName)GetProcAddress(m, Exception_askFileName_FULLNAME);
		if(Exception_askFileName_original == NULL)
		{
			ERRLOG("GetProcAddress(" Exception_askFileName_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" Exception_askFileName_FULLNAME ") returns %p)", Exception_askFileName_original);
		}
	}
	CALL_NEXT(ret = Exception_askFileName_original(obj));
	CALL_END("Exception_askFileName(obj='%p') returns '%s'", obj, ret);
	return ret;
}

/*
	Exception_askLastException
	public: static class UGS::Error::Exception const * __ptr64 __cdecl UGS::Error::Exception::askLastException(void)
*/
static pfException_askLastException Exception_askLastException_original;
UException_PTR Exception_askLastException()
{
	UException_PTR ret = NULL;
	if(Exception_askLastException_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		Exception_askLastException_original = (pfException_askLastException)GetProcAddress(m, Exception_askLastException_FULLNAME);
		if(Exception_askLastException_original == NULL)
		{
			ERRLOG("GetProcAddress(" Exception_askLastException_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" Exception_askLastException_FULLNAME ") returns %p)", Exception_askLastException_original);
		}
	}
	CALL_NEXT(ret = Exception_askLastException_original());
	CALL_END("Exception_askLastException() returns '%p'", ret);
	return ret;
}

/*
	Exception_askLineNumber
	public: int __cdecl UGS::Error::Exception::askLineNumber(void)const __ptr64
*/
static pfException_askLineNumber Exception_askLineNumber_original;
int Exception_askLineNumber(UException_PTR obj)
{
	int ret = 0;
	if(Exception_askLineNumber_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		Exception_askLineNumber_original = (pfException_askLineNumber)GetProcAddress(m, Exception_askLineNumber_FULLNAME);
		if(Exception_askLineNumber_original == NULL)
		{
			ERRLOG("GetProcAddress(" Exception_askLineNumber_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" Exception_askLineNumber_FULLNAME ") returns %p)", Exception_askLineNumber_original);
		}
	}
	CALL_NEXT(ret = Exception_askLineNumber_original(obj));
	CALL_END("Exception_askLineNumber(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	Exception_askSyslogMessage
	public: char const * __ptr64 __cdecl UGS::Error::Exception::askSyslogMessage(void)const __ptr64
*/
static pfException_askSyslogMessage Exception_askSyslogMessage_original;
char const * Exception_askSyslogMessage(UException_PTR obj)
{
	char const * ret = NULL;
	if(Exception_askSyslogMessage_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		Exception_askSyslogMessage_original = (pfException_askSyslogMessage)GetProcAddress(m, Exception_askSyslogMessage_FULLNAME);
		if(Exception_askSyslogMessage_original == NULL)
		{
			ERRLOG("GetProcAddress(" Exception_askSyslogMessage_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" Exception_askSyslogMessage_FULLNAME ") returns %p)", Exception_askSyslogMessage_original);
		}
	}
	CALL_NEXT(ret = Exception_askSyslogMessage_original(obj));
	CALL_END("Exception_askSyslogMessage(obj='%p') returns '%s'", obj, ret);
	return ret;
}

/*
	Exception_askType
	public: enum ERROR_exception_type_t __cdecl UGS::Error::Exception::askType(void)const __ptr64
*/
static pfException_askType Exception_askType_original;
CPP_ENUM Exception_askType(UException_PTR obj)
{
	CPP_ENUM ret = 0;
	if(Exception_askType_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		Exception_askType_original = (pfException_askType)GetProcAddress(m, Exception_askType_FULLNAME);
		if(Exception_askType_original == NULL)
		{
			ERRLOG("GetProcAddress(" Exception_askType_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" Exception_askType_FULLNAME ") returns %p)", Exception_askType_original);
		}
	}
	CALL_NEXT(ret = Exception_askType_original(obj));
	CALL_END("Exception_askType(obj='%p') returns '%d'", obj, ret);
	return ret;
}

/*
	Exception_askUserMessage
	public: char const * __ptr64 __cdecl UGS::Error::Exception::askUserMessage(void)const __ptr64
*/
static pfException_askUserMessage Exception_askUserMessage_original;
char const * Exception_askUserMessage(UException_PTR obj)
{
	char const * ret = NULL;
	if(Exception_askUserMessage_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		Exception_askUserMessage_original = (pfException_askUserMessage)GetProcAddress(m, Exception_askUserMessage_FULLNAME);
		if(Exception_askUserMessage_original == NULL)
		{
			ERRLOG("GetProcAddress(" Exception_askUserMessage_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" Exception_askUserMessage_FULLNAME ") returns %p)", Exception_askUserMessage_original);
		}
	}
	CALL_NEXT(ret = Exception_askUserMessage_original(obj));
	CALL_END("Exception_askUserMessage(obj='%p') returns '%s'", obj, ret);
	return ret;
}

/*
	UString_utf8_str
	public: char const * __ptr64 __cdecl UGS::UString::utf8_str(void)const __ptr64
*/
static pfUString_utf8_str UString_utf8_str_original;
char const * UString_utf8_str(UString_PTR obj)
{
	char const * ret = NULL;
	if(UString_utf8_str_original == NULL)
	{
		auto m = GetModuleHandle("libsyss.dll");
		if(m == NULL)
		{
			ERRLOG("GetModuleHandle('libsyss.dll') Failed(Error=%lu)", GetLastError());
			return ret;
		}
		UString_utf8_str_original = (pfUString_utf8_str)GetProcAddress(m, UString_utf8_str_FULLNAME);
		if(UString_utf8_str_original == NULL)
		{
			ERRLOG("GetProcAddress(" UString_utf8_str_FULLNAME ") Failed(Error=%lu)", GetLastError());
			return ret;
		}
		else
		{
			DBGLOG("GetProcAddress(" UString_utf8_str_FULLNAME ") returns %p)", UString_utf8_str_original);
		}
	}
	CALL_NEXT(ret = UString_utf8_str_original(obj));
	CALL_END("UString_utf8_str(obj='%p') returns '%s'", obj, ret);
	return ret;
}


