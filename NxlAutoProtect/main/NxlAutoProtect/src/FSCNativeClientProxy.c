#include <FSCNativeClientProxy.h>
#include <dce_log.h>
#include <dce_mem.h>


#define DLL_FSCClientProxy "FSCNativeClientProxy"
#define API_FSC_ReadBytes_UTF "FSC_ReadBytes_UTF"
typedef int(*pfFSC_ReadBytes_UTF)(
	const char* policy,
	const char* readTicket,
	const INT64 fileOffset,
	const int bytesToRead,
	void* buffer,
	char **szErrorUTF8);
static pfFSC_ReadBytes_UTF FSC_ReadBytes_UTF_original = NULL;

#ifdef WNT
#include <Windows.h>
HMODULE find_fscclientproxy() {
	static HMODULE mFscClientProxy = NULL;
	if (mFscClientProxy == NULL)
	{
		char dllName[MAX_PATH] = "";
		int vsVersions[] = { 
			160, //TC 13.x is compiled with msvs 2019
			150, //TC 12.x is compiled with visual studio 2017
			140, //TC 12.x FMS is compiled with visual studio 2015,
			120, //TC 11.X FMS is compiled with visual studio 2013
			110, //TC 11.X is compiled with visual studio 2012
			100, //TC 10.X is compiled with visual studio 2010
			90 };
		int nversions = sizeof(vsVersions) / sizeof(int);
		int i;
		for (i = 0; i < nversions; i++) {
			sprintf_s(dllName, MAX_PATH, "%sv%d64.dll", DLL_FSCClientProxy, vsVersions[i]);
			mFscClientProxy = GetModuleHandle(dllName);
			if (mFscClientProxy != NULL) {
				DBGLOG("Found %s=%p", dllName, mFscClientProxy);
				return mFscClientProxy;
			}
			//DBGLOG("Cannot Found %s", dllName);
		}
		DBGLOG(DLL_FSCClientProxy " is not loaded yet.");
	}
	return mFscClientProxy;
}

int FSC_ReadBytes_UTF(
	const char* policy,
	const char* readTicket,
	const INT64 fileOffset,
	const int bytesToRead,
	void* buffer,
	char **szErrorUTF8)
{
	int ret;
	DBGLOG("policy='%s' readTicket='%s' offset=%lu bytesToRead=%d", policy, readTicket, fileOffset, bytesToRead);
	if (FSC_ReadBytes_UTF_original == NULL) {
		//TODO:
		HMODULE module = find_fscclientproxy();
		if (module != NULL)
		{
			FSC_ReadBytes_UTF_original = (pfFSC_ReadBytes_UTF)GetProcAddress(module, API_FSC_ReadBytes_UTF);
		}
		DBGLOG(DLL_FSCClientProxy "=%p " API_FSC_ReadBytes_UTF "=%p", module, FSC_ReadBytes_UTF_original);
		if (FSC_ReadBytes_UTF_original == NULL)
		{
			*szErrorUTF8 = TC_DUP("failed to locate " API_FSC_ReadBytes_UTF);
			return -1;
		}
	}
	ret = FSC_ReadBytes_UTF_original(policy, readTicket, fileOffset, bytesToRead, buffer, szErrorUTF8);
	DBGLOG("policy='%s' readTicket='%s' offset=%lu bytesToRead=%d ret=%d", policy, readTicket, fileOffset, bytesToRead, ret);
	return ret;
}
#endif
#ifdef __linux__
#include <dlfcn.h>

int FSC_ReadBytes_UTF(
	const char* policy,
	const char* readTicket,
	const INT64 fileOffset,
	const int bytesToRead,
	void* buffer,
	char **szErrorUTF8)
{
	int ret;
	DBGLOG("policy='%s' readTicket='%s' offset=%lu bytesToRead=%d", policy, readTicket, fileOffset, bytesToRead);
	if (!FSC_ReadBytes_UTF_original) {
		void *handle;
		char *error;
#define SO_NAME "lib" DLL_FSCClientProxy "64.so"
		handle = dlopen(SO_NAME, RTLD_LAZY);
		error = dlerror();
		if (!handle)
		{
			*szErrorUTF8 = TC_DUP(error);
			ERRLOG("failed to open " SO_NAME ":error=%s", *szErrorUTF8);
			return -1;
		}
		DBGLOG(SO_NAME "=%p", handle);
		FSC_ReadBytes_UTF_original = (pfFSC_ReadBytes_UTF)dlsym(handle, API_FSC_ReadBytes_UTF);
		/* https://man7.org/linux/man-pages/man3/dlopen.3.html
		According to the ISO C standard, casting between function
              pointers and 'void *', as done above, produces undefined results.
              POSIX.1-2001 and POSIX.1-2008 accepted this state of affairs and
              proposed the following workaround:

				*(void **)(&FSC_ReadBytes_UTF_original) = dlsym(handle, API_FSC_ReadBytes_UTF);

              This (clumsy) cast conforms with the ISO C standard and will
              avoid any compiler warnings.

              The 2013 Technical Corrigendum 1 to POSIX.1-2008 improved matters
              by requiring that conforming implementations support casting
              'void *' to a function pointer.  Nevertheless, some compilers
              (e.g., gcc with the '-pedantic' option) may complain about the
              cast used in this program. */
		error = dlerror();
		dlclose(handle);
		if (error)
		{
			*szErrorUTF8 = TC_DUP(error);
			ERRLOG("failed to lcoate " API_FSC_ReadBytes_UTF ":error=%s", *szErrorUTF8);
			return -1;			
		}
		DBGLOG(API_FSC_ReadBytes_UTF "=%p", FSC_ReadBytes_UTF_original);
	}
	ret = FSC_ReadBytes_UTF_original(policy, readTicket, fileOffset, bytesToRead, buffer, szErrorUTF8);
	DBGLOG("policy='%s' readTicket='%s' offset=%lu bytesToRead=%d ret=%d", policy, readTicket, fileOffset, bytesToRead, ret);
	return ret;
}
#endif