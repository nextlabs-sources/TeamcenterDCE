#pragma once

//the original api address which has not be hooked
#define VAR_ORIGINAL(api) api##_original
//the original api address which has be hooked
#define VAR_NEXT(api) api##_next
//the customized api address which is going to replace original api
#define VAR_OURS(api) api##_hooked

#define VAR_FULLNAME(api) api##_FULLNAME

#define DEFINE_HOOK_CALLBACK(api) \
static pf##api api##_original = NULL; \
static pf##api api##_next = NULL; \
static const char* api##_FULLNAME = _STRINGIZE(api); \
static bool api##InHook = false;

//check the status of the api hook
#define IS_HOOKED(api) (NULL != VAR_NEXT(api))
#define NOT_HOOKED(api) (NULL == VAR_NEXT(api))
#define IS_INIT(api) (NULL != VAR_ORIGINAL(api))
#define NOT_INIT(api) (NULL == VAR_ORIGINAL(api))

//unhook api
#define UNHOOK(api)		\
	if( IS_HOOKED(api) && UnhookCode((PVOID*)&VAR_NEXT(api))) {	\
		LOG_INFO_FMT(L"API unhooked: %s(0x%p)", _W(_STRINGIZE(api)), (PVOID)&VAR_NEXT(api)); \
		VAR_NEXT(api) = NULL;	\
	}

//hook api that we know the signature and have the corresponding lib file in which contains the ProcAddres
#define HOOK(api)	\
	if(NOT_HOOKED(api)) {	\
		VAR_ORIGINAL(api) = api;	\
		if(HookCode((PVOID)VAR_ORIGINAL(api), (PVOID)VAR_OURS(api), (PVOID*)&VAR_NEXT(api), 0))	{ \
			LOG_INFO_FMT(L"API hooked: %s(0x%p)", _W(_STRINGIZE(api)), (PVOID)&VAR_NEXT(api)); \
		} \
	}

//hook API by module name, this is used for the dll which is not loaded on NX starts
#define HOOK_API(libName, apiName)	\
	if(NOT_HOOKED(apiName)) { \
		if(HookAPI(libName, VAR_FULLNAME(apiName), (PVOID)VAR_OURS(apiName), (PVOID*)&VAR_NEXT(apiName), 0)) {	\
			LOG_INFO_FMT(L"API hooked: %s %s(0x%p)", _W(_STRINGIZE(libName)), _W(_STRINGIZE(apiName)), (PVOID)&VAR_NEXT(apiName)); \
		} else {	\
			LOG_INFO_FMT(L"Failed to hook API(error: %lu): %s %s", GetLastError(), _W(_STRINGIZE(libName)), _W(_STRINGIZE(apiName))); \
		}	\
	}

/*
CreateFileA and CreateFileW
*/
typedef HANDLE(WINAPI* pfCreateFileA)(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

typedef HANDLE(WINAPI* pfCreateFileW)(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

/*
mapi32!MAPISendEmail
*/
typedef ULONG(WINAPI* pfMAPISendMail)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	_In_ lpMapiMessage lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
	);

typedef ULONG(WINAPI* pfMAPISendMailW)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	_In_ lpMapiMessageW lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
	);