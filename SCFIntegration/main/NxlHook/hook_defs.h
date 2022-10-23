#ifndef NXL_HOOK_DEFS_H_INCLUDED
#define NXL_HOOK_DEFS_H_INCLUDED
#include <Windows.h>
#include <madCHook.h>

#define CONCAT(a, b) a##b

#define WCHAR(txt) CONCAT(L, txt)
/*
	Name convention for API hooks
*/
//the function pointer that represent the api singature
#define TYPE(api) pf##api
//the original api address which has not be hooked
#define VAR_ORIGINAL(api) api##_original
//the original api address which has be hooked
#define VAR_NEXT(api) api##_next
//the customized api address which is going to replace original api
#define VAR_OURS(api) api##_hooked

//check the status of the api hook
#define IS_HOOKED(api) (NULL != VAR_NEXT(api))
#define NOT_HOOKED(api) (NULL == VAR_NEXT(api))
#define IS_INIT(api) (NULL != VAR_ORIGINAL(api))
#define NOT_INIT(api) (NULL == VAR_ORIGINAL(api))

//unhook api
#define UNHOOK(api)		\
	if( IS_HOOKED(api) && UnhookCode((PVOID*)&VAR_NEXT(api))) {	\
		DBGLOG(#api":Removed hook-"STRINGIFY(VAR_NEXT(api))"(%p)", VAR_NEXT(api));	\
		VAR_NEXT(api) = NULL;	\
	}

//hook api that we know the signature and have the corresponding lib file in which contains the ProcAddres
#define HOOK(api)	\
	if(NOT_HOOKED(api)) {	\
		VAR_ORIGINAL(api) = api;	\
		if(HookCode((PVOID)VAR_ORIGINAL(api), (PVOID)VAR_OURS(api), (PVOID*)&VAR_NEXT(api), 0))	\
			DBGLOG("Hook:'"#api"' \tNEXT=%p", (PVOID)VAR_NEXT(api));			\
	}

//hook C style api that we know the signature but the lib file
#define HOOK_byName(apiName, m)	\
	if(NOT_HOOKED(apiName) && m!=NULL) {	\
		VAR_ORIGINAL(apiName) = (TYPE(apiName))GetProcAddress(m, #apiName);	\
		if(NOT_INIT(apiName)) {	\
			DBGLOG(#apiName":Failed to GetProcAddress in module-'%d'(%d) by name", m, GetLastError());	\
		} else if(HookCode((PVOID)VAR_ORIGINAL(apiName), (PVOID)VAR_OURS(apiName), (PVOID*)&VAR_NEXT(apiName), 0)) {	\
			DBGLOG("Hook:'"#apiName"' \tNEXT=%p", (PVOID)VAR_NEXT(apiName));			\
		} else {	\
			DBGLOG(#apiName":Failed to HookCode(NEXT=%p)", (PVOID)VAR_NEXT(apiName));	\
		}	\
	}

//this is Abandoned
////initialized the invoke address of C++ mangling api by full name or ordinal
//#define INIT_API_CPP(apiName, m)	\
//	if(NOT_INIT(apiName)) {		\
//		VAR_ORIGINAL(apiName) = (TYPE(apiName))GetProcAddress(m, apiName##_FULLNAME);	\
//		if(NOT_INIT(apiName)) {		\
//			DBGLOG(#apiName":Failed(%d) to GetProcAddress(%p, '"apiName##_FULLNAME"')", GetLastError(), m);	\
//			VAR_ORIGINAL(apiName) = (TYPE(apiName))GetProcAddress(m, MAKEINTRESOURCE(apiName##_ORDINAL));	\
//			if(NOT_INIT(apiName)) {	\
//				DBGLOG(#apiName":Failed(%d) to GetProcAddress(%p, %d)", GetLastError(), m, apiName##_ORDINAL);	\
//			}	\
//		}	\
//		if(IS_INIT(apiName)) {	\
//			DBGLOG("Init:'"#apiName"'@%p \tProcAddress=%p", m, (PVOID)VAR_ORIGINAL(apiName));			\
//		}	\
//	}

//hook C++ mangling api
#define HOOK_CPP(apiName, m)	\
	if(NOT_HOOKED(apiName) && m!=NULL) {	\
		if(NOT_INIT(apiName)) {		\
			VAR_ORIGINAL(apiName) = (TYPE(apiName))GetProcAddress(m, apiName##_FULLNAME);	\
			if(NOT_INIT(apiName)) {		\
				DBGLOG(#apiName":Failed(%d) to GetProcAddress(%p, '"apiName##_FULLNAME"')", GetLastError(), m);	\
				VAR_ORIGINAL(apiName) = (TYPE(apiName))GetProcAddress(m, MAKEINTRESOURCE(apiName##_ORDINAL));	\
				if(NOT_INIT(apiName)) {	\
					DBGLOG(#apiName":Failed(%d) to GetProcAddress(%p, %d)", GetLastError(), m, apiName##_ORDINAL);	\
				}	\
			}	\
		}	\
		if(IS_INIT(apiName)) {	\
			if(HookCode((PVOID)VAR_ORIGINAL(apiName), (PVOID)VAR_OURS(apiName), (PVOID*)&VAR_NEXT(apiName), 0)) {	\
				DBGLOG("Hook:'"#apiName"'@%p \tOriginal=%p NEXT=%p", m, VAR_ORIGINAL(apiName), (PVOID)VAR_NEXT(apiName));			\
			} else if(VAR_NEXT(apiName) != NULL && VAR_NEXT(apiName) != VAR_ORIGINAL(apiName)) {	\
				DBGLOG("Hook:'"#apiName"'@%p \tOriginal=%p NEXT=%p Error=%lu", m, VAR_ORIGINAL(apiName), (PVOID)VAR_NEXT(apiName), GetLastError());			\
			} else {	\
				DBGLOG("Hook:'"#apiName"'@%p FAILED(Error=%lu):Original=%p", m, GetLastError(), VAR_ORIGINAL(apiName));	\
			}	\
		}	\
	}

//hook API by module name, this is used for the dll which is not loaded on NX starts
#define HOOK_API(mName, apiName)	\
	if(NOT_HOOKED(apiName)) {	\
		if(HookAPI(mName, apiName##_FULLNAME, (PVOID)VAR_OURS(apiName), (PVOID*)&VAR_NEXT(apiName), 0)) {	\
			DBGLOG("Hook:'"#apiName"'@%s \tNEXT=%p", mName, (PVOID)VAR_NEXT(apiName));			\
		} else if(VAR_NEXT(apiName) != NULL && VAR_NEXT(apiName) != VAR_ORIGINAL(apiName)) {	\
			DBGLOG("Hook:'"#apiName"'@%s \tNEXT=%p Error=%lu", mName, (PVOID)VAR_NEXT(apiName), GetLastError());			\
		} else {	\
			DBGLOG("Hook:'"#apiName"'@%s FAILED(Error=%lu)", mName, GetLastError());	\
		}	\
	}

//for C++ member functions, the first parameter is the owner object
typedef const void * CPP_PTR;
typedef int CPP_ENUM;
typedef const void * CALLBACK_PTR; //
#endif