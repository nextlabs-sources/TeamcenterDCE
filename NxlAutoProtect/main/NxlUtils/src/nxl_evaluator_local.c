#ifdef WNT
#include <utils_windows.h>
#include "nxl_evaluator_local.h"
#include "CEsdk.h"
#include "utils_log.h"
#include "utils_string.h"
#include "utils_nxl.h"
#include "utils_mem.h"
#include <internal.h>

#ifdef WIN64
#define CESDK_DLL "cesdk.dll"
#else
#define CESDK_DLL "cesdk32.dll"
#endif

#define PC_IS_UP 1
#define PC_IS_DOWN 0

#define EVAL_TIMEOUT (30 * 1000)
#define EVAL_NOISE_LEVEL CE_NOISE_LEVEL_USER_ACTION

extern int obligation_load(string_list_ro list, obligation_list *obligations);

int check_pc_status(void)
{
	HANDLE pc;
	CALL_DTL(pc = OpenFileMapping(0x0002, 0, "Global\\b67546e2-6dc7-4d07-aa8a-e1647d29d4d7"));
	if(pc != NULL)
	{
		CALL_DTL(CloseHandle(pc));
		return PC_IS_UP;
	}
	else
	{
		return PC_IS_DOWN;
	}
}

void  free_attributes(CEAttributes *ceAttrs)							
{
	if(ceAttrs != NULL)
	{
		int i;											
		for(i = 0; i< ceAttrs->count; i++)				
		{											
			CEAttribute attr = ceAttrs->attrs[i];		
			CALL_DTL(CEM_FreeString(attr.key));
			CALL_DTL(CEM_FreeString(attr.value));			
		}
		NXL_FREE(ceAttrs->attrs);
	}
	NXL_FREE(ceAttrs);
}
size_t string_from_cestring(char **nstr, CEString ceStr)
{
	size_t convertedCnt = 0;
	wchar_t *wchars = (wchar_t *)CEM_GetString(ceStr);
	*nstr = NULL;
	if(wchars != NULL)
	{
		size_t wstrLen = wcslen(wchars);
		if(wstrLen > 0)
		{
			size_t strLen = (wstrLen + 1) * 2;
			char *nstring;
			NXL_ALLOCN(nstring, char, strLen);
			CALL_DTL(wcstombs_s(&convertedCnt, nstring, strLen, wchars, _TRUNCATE));
			DTLLOG("Converted CEString to '%s'(%d)", nstring, convertedCnt);
			*nstr = nstring;
		}
	}
	return convertedCnt;
}

CEString string_to_cestring(const char *str)
{
	CEString cestr = NULL;
	if(str != NULL)
	{
		size_t strLen = strlen(str) + 1;
		size_t converted;
		wchar_t *wstr;
		NXL_ALLOCN(wstr, wchar_t, strLen);

		CALL_DTL(mbstowcs_s(&converted, wstr, strLen, str, _TRUNCATE));
		DTLLOG("Converted '%s' to CEString(%d)", str, converted);
		cestr = CEM_AllocateString((char *)wstr);
		NXL_FREE(wstr);
	}
	return cestr;
}


BOOL load_cesdk()
{
	HMODULE module;
	if((module = GetModuleHandle(CESDK_DLL)) == NULL)
	{
		const char *keyval = NULL;
		if ((keyval = REG_get("SOFTWARE\\NextLabs\\CommonLibraries", "InstallDir")) != NULL)
		{
			char lszValue[MAX_PATH];
			strcpy_s(lszValue, MAX_PATH, keyval);
#ifdef WIN64
			CALL_DBG(strncat_s(lszValue, MAX_PATH, "\\bin64\\", _TRUNCATE));
#else
			CALL_DBG(strncat_s(lszValue, MAX_PATH, "\\bin32\\", _TRUNCATE));
#endif
			if(SetDllDirectory(lszValue))
			{
				DBGLOG("SetDllDirectory('%s'):Success", lszValue);
			}
			else
			{
				DBGLOG("SetDllDirectory('%s'):Error=%#X", lszValue, GetLastError());
			}
		}
		module = LoadLibrary(CESDK_DLL);
		SetDllDirectory(NULL);
	}
	if(module != NULL)
	{
		char buffer[MAX_PATH];
		if(GetModuleFileName(module, buffer, MAX_PATH) > 0)
		{
			DBGLOG(CESDK_DLL" is loaded(Path='%s')", buffer);
		}
		else
		{
			DBGLOG(CESDK_DLL" is loaded(Path='%s'), but Failed to GetModuleFileName!(ErrorCode:%#X)", buffer, GetLastError());
		}
		return TRUE;
	}
	else
	{
		ERRLOG("Failed to load %s", CESDK_DLL);
		return FALSE;
	}
}
eval_result_t local_evaluate( const char *action,
	eval_app_ro evalApp, eval_user_ro evalUser, eval_res_ro evalSrc, 
	obligation_list *obligations, int *nob)
{
	eval_result_t retVal = EVAL_DEFAULT;
	DBGLOG("Evaluating Action-%s on local PC", action);
	if(obligations != NULL)	*obligations = NULL;
	if(nob != NULL) *nob = 0;
	if(string_isNullOrSpace(action))
	{
		DBGLOG("Action is not properly set!");
	}
	else if(evalApp==NULL)
	{
		DBGLOG("Application Attributes is not properly initialized!");
	}
	else if(evalUser==NULL)
	{
		DBGLOG("User Attributes is not properly initialized!");
	}
	else if(evalSrc==NULL)
	{
		DBGLOG("Source Resource is not Properly initialized!");
	}
	else
	{
		CEHandle cnnHndl = NULL;
		CEApplication app;
		CEUser user;
		CEAttributes *userAttrs = NULL;
		CEResource source;
		CEAttributes *sourceAttrs = NULL;
		CEResult_t result;	

		if(check_pc_status() != PC_IS_UP)
		{
			ERRLOG("PC is DOWN or NOT installed!");
			return EVAL_DEFAULT;
		}
		if(!load_cesdk())
		{
			return EVAL_DEFAULT;
		}
		//Initialize CEApplication
		DBGLOG("Initializing App Infos(Name=%s Url=%s)...", evalApp->name, evalApp->url);
		app.appName = string_to_cestring(evalApp->name);
		app.appPath = NULL;//MUST HAVE
		app.appURL = string_to_cestring(evalApp->url);//MUST HAVE

		DBGLOG("Initializing User Infos(Name=%s ID=%s)...", evalUser->name, evalUser->id);
		{
			user.userName = string_to_cestring(evalUser->name);
			user.userID = string_to_cestring(evalUser->id);

			if(evalUser->attributes!=NULL)
			{
				DBGLOG("Initializing User Attributes(%d)...", evalUser->attributes->count);
				{
					CEAttribute *uAttrs = NULL;
					int nAttr = evalUser->attributes->count;
					if(evalUser->attributes->count > 0)
					{
						int iAttr;
						NXL_ALLOCN(uAttrs, CEAttribute, nAttr);
						for(iAttr = 0; iAttr < nAttr; iAttr++)
						{
							uAttrs[iAttr].key = string_to_cestring(evalUser->attributes->keys[iAttr]);
							uAttrs[iAttr].value = string_to_cestring(evalUser->attributes->vals[iAttr]);
						}
					}
					NXL_ALLOC(userAttrs, CEAttributes);
					userAttrs->attrs = uAttrs;
					userAttrs->count = nAttr;
				}
			}
		}

		DBGLOG("Initializing Source Infos(Name=%d Type=%s)...", evalSrc->name, evalSrc->type);
		{
			source.resourceName = string_to_cestring(evalSrc->name);
			source.resourceType = string_to_cestring(evalSrc->type);
		}
	
		DBGLOG("Initializing Source Attributes(%d)...", evalSrc->attributes->count);
		{
			int iAttr, iCnt=0;
			CEAttribute *sattrs;
			NXL_ALLOCN(sattrs, CEAttribute, evalSrc->attributes->count);
			for(iAttr = 0; iAttr < evalSrc->attributes->count; iAttr++, iCnt++)
			{
				sattrs[iAttr].key = string_to_cestring(evalSrc->attributes->keys[iAttr]);
				sattrs[iAttr].value = string_to_cestring(evalSrc->attributes->vals[iAttr]);
			}
			NXL_ALLOC(sourceAttrs, CEAttributes);
			sourceAttrs->attrs = sattrs;
			sourceAttrs->count = evalSrc->attributes->count;
		}

		//initialize connection handler
		CALL_DBG(result = CECONN_Initialize(app, user, NULL, &cnnHndl, EVAL_TIMEOUT));
		if(result != CE_RESULT_SUCCESS
			|| cnnHndl == NULL)
		{
			ERRLOG("Failed to create connection(%d) to PC(Error Code:%d)", cnnHndl, result);
			goto CLEANUP;
		}
		DBGLOG("Connection Handler Created(%d)", cnnHndl);
		{
			CEBoolean performObligation = CETrue;
			CEString evalOp = string_to_cestring(action);
			CEEnforcement_t enforcement;
			CEint32 ipNumber = IP_local_number();

			//call evaluate
			CALL_DBG(
				result = CEEVALUATE_CheckResources(
							cnnHndl,			//CEHandle - connection handler
							evalOp,		//CEString - operation in CEString type
							&source,			//CEResource* - the source resource in CEResource type
							sourceAttrs,	//CEAttributes* - associate attributes of the source
							NULL,				//CEResource*  - the target resource
							NULL,				//CEAttributes* - associate atributes of the source
							&user,				//CEUser - the user who access the resources
							userAttrs,			//CEAttributes* - Associate attributes of the user
							&app,				//CEApplication* - the application which access this resource
							NULL,				//CEAttributes* - ssociate attributes of the application
							NULL,				//CEString* - The string array of recipients for the messaging case
							0,					//CEint32 - The number of recipients for the messaging case
							ipNumber,			//CEint32 - the IP address of client machine
							performObligation,	//CEBoolean - perform the obligation defined by the policy
							EVAL_NOISE_LEVEL,	//CENoiseLevel_t - desirable noise level to be used for this evaluation
							&enforcement,		//CEEnforcement_t* - result of the policy for enforcement
							EVAL_TIMEOUT));		//CEint32 - desirable timeout in milliseconds for evaluation

			if(result != CE_RESULT_SUCCESS)
			{
				ERRLOG("Evaluation Failed(%d)", result);
				goto CLEANUP;
			}
			if(enforcement.result == CEDeny)//do translation when Allow
			{
				SYSLOG("Eval Result:Deny");
				retVal = EVAL_DENY;
			}
			else
			{
				SYSLOG("Eval Result:Allow");
				retVal = EVAL_ALLOW;
			}
			//load obligations
			if(obligations == NULL || nob == NULL)
			{
				DBGLOG("Skipped loading obligations");
			}
			else if(enforcement.obligation != NULL
				&& enforcement.obligation->count > 0)
			{
				int attrCnt = (enforcement.obligation->count)*2;
				int i;
				string_list_p list = string_list_new();
				DBGLOG("Obligation Attr Count=%d Array Size=%d", enforcement.obligation->count, attrCnt);
				//convert CEAttributes to char **
				for(i = 0; i< enforcement.obligation->count; i++)
				{
					char *k, *v;
					CEAttribute attr = (enforcement.obligation->attrs)[i];
					CALL_DTL(string_from_cestring(&k, attr.key));
					CALL_DTL(string_from_cestring(&v, attr.value));
					DTLLOG("[%d]%s=%s", i, k, v);
					string_list_add_internal(list, k);
					string_list_add_internal(list, v);
				}
				CALL_DBG(*nob = obligation_load(list, obligations));
				//SYSLOG("Found %d DRM obligations in enforcement", *nob);
				string_list_free(list);
			}

			//free enforcement.obligation
			CALL_DTL(CEEVALUATE_FreeEnforcement(enforcement));
			CALL_DTL(CEM_FreeString(evalOp));
		}

	CLEANUP:
		DBGLOG("Finishing...");
		//close connection handler
		if(cnnHndl != NULL)
		{
			CALL_DBG(CECONN_Close(cnnHndl, EVAL_TIMEOUT));
		}
		//NXL_FREE
		CALL_DTL(CEM_FreeString(app.appName));

		CALL_DTL(CEM_FreeString(user.userName));
		CALL_DTL(CEM_FreeString(user.userID));
		CALL_DTL(free_attributes(userAttrs));

		CALL_DTL(CEM_FreeString(source.resourceName));
		CALL_DTL(CEM_FreeString(source.resourceType));
		CALL_DTL(free_attributes(sourceAttrs));
	}
	return retVal;
}
#endif