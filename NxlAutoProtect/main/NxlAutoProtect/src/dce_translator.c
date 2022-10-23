#include "dce_translator.h"
#include "dce_log.h"
#include "dce_mem.h"
#include "utils_string.h"
#include <utils_system.h>
#include <key_value_list.h>
#include <dispatcher/dispatcher_itk.h> //DISPATCHER_create_request
#include <tccore/aom_prop.h>

//Translation Request Args
#define PROVIDER_NAME ( "NEXTLABS" ) //For translation request
#define SERVICE_NAME ( "tonxlfile" ) //For translation request
#define REQUEST_TYPE ( "tonxlfile" ) //For translation request
#define DATA_FILE_KEY "MetadataFile"
#define DATA_FILE_PREFIX TEXT("RMX")

#define TRANSLATION_TYPE	"nxlaction"
#define TRANSLATION_REMARK	"nxlremark"

#define TRANSLATION_TYPE_PROTECT	"protect"
#define TRANSLATION_TYPE_UPDATE	"updateTag"
#define TRANSLATION_TYPE_REMOVE "unprotect"
#define TRANSLATION_TYPE_REFRESH_DUID "refreshduid"

#define DELETED(x) (x > 0)
#define NO_DUPLICATES(x) (x == 0)
#define SKIPPED(x) (x < 0)
int check_duplicates(tag_info_ro dsInfo, tag_info_ro revInfo, const char *action)
{
	int nproviders = 1;
	const char *providers[] = {PROVIDER_NAME};
	int nservices = 1;
	const char *services[] = {SERVICE_NAME};
	const char *states[] = {"INITIAL"};
	int nstates = 0;//sizeof(states)/sizeof(char*);
	int priorities[] = {priority_low, priority_medium, priority_high};
	int npriorities = 0;//sizeof(priorities)/sizeof(int);
	int nrequests;
	int ndeleted = 0;
	tag_t *requests;
	if(ITK_EVAL(DISPATCHER_query_requests(
		nproviders, providers,		//providers
		nservices, services,		//services
		nstates, states,			//states
		npriorities, priorities,	//priorities
		0, NULL,					//users
		0, NULL,					//tasks ids
		&nrequests, &requests)))
	{
		int irequest;
		DBGLOG("Found %d requests.", nrequests);
		for(irequest=0; irequest<nrequests; irequest++)
		{
			int isFinalState = FALSE;
			DBGLOG("[%d/%d]:Tag=%d", irequest+1, nrequests, requests[irequest]);
			if(ITK_EVAL(DISPATCHER_is_final_state(requests[irequest], &isFinalState)))
			{
				char *taskId, *providerName, *serviceName, *state, *type;
				tag_t *primaryObjs, *secondaryObjs;
				char **argKeys, **argData;
				int priority, numObjs, numArgs;
				logical isTargetMatched = false;
				logical isActionMatched = false;
				logical isInitial = false;
				DBGLOG("==>isFinalState=%d", isFinalState);
				if(!isFinalState
					&& ITK_EVAL(DISPATCHER_find_request_by_tag(requests[irequest], &taskId, &providerName, &serviceName,
						&priority, &state, &type,
						&numObjs, &primaryObjs, &secondaryObjs,
						&numArgs, &argKeys, &argData)))
				{
					int i;
					NEED_FREE(taskId);
					NEED_FREE(providerName);
					NEED_FREE(serviceName);
					NEED_FREE(state);
					NEED_FREE(type);
					NEED_FREE(primaryObjs);
					NEED_FREE(secondaryObjs);
					NEED_FREE_N(argKeys, numArgs);
					NEED_FREE_N(argData, numArgs);
					DBGLOG("==>TaskId=%s", taskId);
					TC_FREE(taskId);
					DBGLOG("==>ProviderName=%s", providerName);
					TC_FREE(providerName);
					DBGLOG("==>ServiceName=%s", serviceName);
					TC_FREE(serviceName);
					DBGLOG("==>Priority=%d", priority);
					//state
					DBGLOG("==>State=%s", state);
					if(_stricmp(state, "INITIAL")==0)
					{
						isInitial = true;
					}
					TC_FREE(state);
					//type
					DBGLOG("==>Type=%s", type);
					TC_FREE(type);
					//print objects
					for(i=0; i<numObjs; i++)
					{
						DBGLOG("==>Objs[%d/%d]Primary=%d Secondary=%d", i+1, numObjs, primaryObjs[i], secondaryObjs[i]);
					}
					if(numObjs==1
						&& primaryObjs[0]==dsInfo->tagId
						&& (revInfo==NULL||secondaryObjs[0]==revInfo->tagId))
					{
						isTargetMatched = true;
						DBGLOG("==>Target Matched~!");
					}
					TC_FREE(primaryObjs);
					TC_FREE(secondaryObjs);
					if(numArgs>0
						&& strcmp(argKeys[0], TRANSLATION_TYPE)==0
						&& strcmp(argData[0], action)==0)
					{
						isActionMatched = true;
						DBGLOG("==>Action Matched~!");
					}
					//print args
					for(i=0; i<numArgs; i++)
					{
						DBGLOG("==>Args[%d/%d]%s=%s", i+1, numArgs, argKeys[i], argData[i]);
						TC_FREE(argKeys[i]);
						TC_FREE(argData[i]);
					}
					TC_FREE(argKeys);
					TC_FREE(argData);

					//check if this request need to be deleted
					if(isTargetMatched 
						&& isActionMatched)
					{
						if(_stricmp(action, TRANSLATION_TYPE_REMOVE)==0)
						{
							//skip unprotect translation when meet duplicates
							DBGLOG("Found duplicated '%s' translation, SKIPPED! ", action);
							ndeleted--;
							break;
						}
						else if(isInitial)
						{
							DBGLOG("Found duplicated '%s' translation which status is initial, DELETE! ", action);
							if(ITK_EVAL(DISPATCHER_delete_request(requests[irequest], TRUE)))
							{
								DBGLOG("==>Deleted");
								ndeleted++;
							}
						}
					}
				}//ITK_EVAL(DISPATCHER_find_request_by_tag(
			}//if(ITK_EVAL(DISPATCHER_is_final_state(requests[irequest], &isFinalState)))
		}//for(irequest=0; irequest<nrequests; irequest++)
		TC_FREE(requests);
	}//if(ITK_EVAL(DISPATCHER_query_requests(
	if(DELETED(ndeleted)) DBGLOG("Deleted %d translation requests", ndeleted);
	return ndeleted;
}

const char *build_remark(const char *reason)
{
	static char remarkBuff[1024] = "";
	int nheader = 0;
	if(nheader == 0)
	{
		//build header
		const char *hostname = host_get_name();
		const char *syslog = EMH_ask_system_log();
		const char *nxllog = log_get_file();
		DBGLOG("Hostname='%s'", hostname);
		DBGLOG("NxlLog='%s'", nxllog);
		DBGLOG("TCsyslog='%s'", syslog);
		nheader = sprintf_s(remarkBuff, 1024, TRANSLATION_REMARK "=HOST-%s;TCLOG-%s;NXLLOG-%s;"
			, hostname
			, PathFindFileName(syslog)
			, PathFindFileName(nxllog));
		DBGLOG("RemarkHeader='%s'", remarkBuff);
	}
	sprintf_s(remarkBuff + nheader, 1024-nheader, "REASON:%s", reason);
	DBGLOG("Remark='%s'", remarkBuff);
	return remarkBuff;
}

tag_t send_translation_request(tag_info_ro dsInfo, tag_info_ro revInfo, kvl_ro additionalTags, const time_t scheduledTime, request_type request, request_priority priority, const char *reason)
{
	//return
	tag_t requestTag = NULLTAG;

	//initialize args
	string_list_p args = string_list_new();
	//build request string
	switch(request)
	{
		case request_update:
			//add translate as default request type
			string_list_add(args, TRANSLATION_TYPE "=" TRANSLATION_TYPE_UPDATE);
			//add props if necessary
			if(pref_send_props_for_update())
			{
				//get all properties of dataset
				hashtable_ro props = tag_info_get_props(dsInfo);
				int i;
				for(i=0; i<props->count; i++)
				{
					const char *key = props->kvps[i].key;
					const char *val = (const char *)props->kvps[i].value;
					DBGLOG(TAG_INFO_FMT "[%d/%d]'%s'='%s'", TAG_INFO_ARGS(dsInfo), i+1, props->count, key, val);
					if(!string_isNullOrSpace(key)
						&& !string_isNullOrSpace(val))
					{
						string_list_add(args, strings_join_v("=", 2, key, val));
					}
				}
			}
			break;
		case request_unprotect:
			//add translate as default request type
			string_list_add(args, TRANSLATION_TYPE "=" TRANSLATION_TYPE_REMOVE);
			break;
		case request_refreshduid:
			string_list_add(args, TRANSLATION_TYPE "=" TRANSLATION_TYPE_REFRESH_DUID);
			break;
		case request_protect:
			//add translate as default request type
			string_list_add(args, TRANSLATION_TYPE "=" TRANSLATION_TYPE_PROTECT);
			//add props if necessary
			if((additionalTags == NULL || additionalTags->count == 0)
				&& pref_send_props_for_protect())
			{
				//add protect tags
				kvl_p tags = dataset_get_protect_attributes(dsInfo);
				int i;
				for(i = 0; i < tags->count; i++)
				{
					DBGLOG(TAG_INFO_FMT "[%d/%d]'%s'='%s'", TAG_INFO_ARGS(dsInfo), i+1, tags->count, tags->keys[i], tags->vals[i]);
					if(!string_isNullOrSpace(tags->keys[i])
						&& !string_isNullOrSpace(tags->vals[i]))
					{
						string_list_add(args, strings_join_v("=", 2, tags->keys[i], tags->vals[i]));
					}
				}
			}
			break;
		default:
			goto CLEAN_UP;
	}
	/*
	if(SKIPPED(check_duplicates(dsInfo, revInfo, requestType)))
	{
		return requestTag;
	}
	*/
	//append additionalTags, normally it's from workflow obligation tags, by default it should be empty
	if(additionalTags != NULL && additionalTags->count > 0)
	{
		int i;
		for(i = 0; i < additionalTags->count; i++)
		{
			DBGLOG("AdditionalTags[%d/%d]'%s'='%s'", i+1, additionalTags->count, additionalTags->keys[i], additionalTags->vals[i]);
			if(!string_isNullOrSpace(additionalTags->keys[i])
				&& !string_isNullOrSpace(additionalTags->vals[i]))
			{
				string_list_add(args, strings_join_v("=", 2, additionalTags->keys[i], additionalTags->vals[i]));
			}
		}
	}

	//build remark
	string_list_add(args, build_remark(reason));

	ITK_CALL(
		DISPATCHER_create_request(
			PROVIDER_NAME,		//char* - provider name
			SERVICE_NAME,		//char* service name
			priority,			//int - priority
			scheduledTime,		//time_t - start time
			0,					//time_t - end time
			0,					//int - repeating interval
			1,					//int - number of objects
			&dsInfo->tagId,		//tag_t* - primary objects
			revInfo!=NULL
				?(&revInfo->tagId)
				:NULL,			//tag_t* - secondary objects
			args->count,				//int - num of args
			(const char **)args->items,				//char** - request args
			REQUEST_TYPE,		//char* - request type string
			0,			//int - number of datafiles
			NULL,			//char** - keys for data files to attach
			NULL,			//char** - data files to attach absolute path
			&requestTag));		//tag_t* - created request

	if(revInfo != NULL)
	{
		SYSLOG("Created Translation Request(%d) for Primary Object" TAG_INFO_FMT " and Secondary Object" TAG_INFO_FMT
			, requestTag, TAG_INFO_ARGS(dsInfo), TAG_INFO_ARGS(revInfo));
	}
	else
	{
		SYSLOG("Created Translation Request(%d) for Primary Object" TAG_INFO_FMT " and Secondary Object=NULL"
			, requestTag, TAG_INFO_ARGS(dsInfo));
	}
	if(revInfo != NULL)
	{
		//when revInfo == NULL, DISPATCHER_find_request_by_tag will crash
		char *taskId, *providerName, *serviceName, *state, *type;
		tag_t *primaryObjs, *secondaryObjs;
		char **argKeys, **argData;
		int priority, numObjs, numArgs;
		if(ITK_EVAL(DISPATCHER_find_request_by_tag(requestTag, &taskId, &providerName, &serviceName
			, &priority, &state, &type,
			&numObjs, &primaryObjs, &secondaryObjs,
			&numArgs, &argKeys, &argData)))
		{
			int i;
			NEED_FREE(taskId);
			NEED_FREE(providerName);
			NEED_FREE(serviceName);
			NEED_FREE(state);
			NEED_FREE(type);
			NEED_FREE(primaryObjs);
			NEED_FREE(secondaryObjs);
			NEED_FREE_N(argKeys, numArgs);
			NEED_FREE_N(argData, numArgs);
			DBGLOG("==>TaskId=%s", taskId);
			TC_FREE(taskId);
			DBGLOG("==>ProviderName=%s", providerName);
			TC_FREE(providerName);
			DBGLOG("==>ServiceName=%s", serviceName);
			TC_FREE(serviceName);
			DBGLOG("==>Priority=%d", priority);
			//state
			DBGLOG("==>State=%s", state);
			TC_FREE(state);
			//type
			DBGLOG("==>Type=%s", type);
			TC_FREE(type);
			//print args
			for(i=0; i<numArgs; i++)
			{
				DBGLOG("==>Args[%d/%d]%s=%s", i+1, numArgs, argKeys[i], argData[i]);
				TC_FREE(argKeys[i]);
				TC_FREE(argData[i]);
			}
			TC_FREE(argKeys);
			TC_FREE(argData);
			//print objects
			for(i=0; i<numObjs; i++)
			{
				DBGLOG("==>Objs[%d/%d]Primary=%d Secondary=%d", i+1, numObjs, primaryObjs[i], secondaryObjs!=NULL?secondaryObjs[i]:NULLTAG);
			}
			TC_FREE(primaryObjs);
			TC_FREE(secondaryObjs);
		}
	}
CLEAN_UP:
	CALL_DTL(string_list_free(args));
	return requestTag;
}


