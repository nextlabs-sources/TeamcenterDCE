#include <pom/pom/pom.h> //POM_get_user(); POM_get_user_id();
#include <stdlib.h>
#include <tc/preferences.h>
#include <tccore/aom_prop.h>
//Nextlabs
#include <utils_string.h>
#include <utils_system.h>
#include <nxl_evaluator_local.h>
#include <nxl_evaluator_remote.h>
//from current project
#include "dce_mem.h"
#include "dce_evaluator.h"
#include "dce_log.h"

//Defines
#define EVAL_APP_NAME PROJECT_NAME


#define DCE_OPERATION_UNPROTECT "RIGHT_EXECUTE_REMOVEPROTECTION"

#define TAGGING_TYPE_NO_ENCRYPTION "No-Encryption"
#define TAGGING_TYPE_ALL_COLUMNS "All-Properties"
#define TAGGING_TYPE_USER_DEFINED "User-Defined"
#define TAGGING_TYPE_SPECIFIC "Specific-Property"
#define TAGGING_TYPE_SYSTEM_COLUMNS "System-Properties"
#define TAGGING_TYPE_AS_PREFERENCE "Preference-Defined"	//TODO:REVIEW
//DLP Evaluation
#define DOWNLOAD_OPERATION "RIGHT_EXECUTE_DOWNLOAD"

const char *get_remote_ip()
{
	static BOOL initialized = FALSE;
	static const char *remoteIP = NULL;
	if(initialized == FALSE)
	{
		const char *ip = pref_get_char(PREF_REMOTE_IP, NULL);
#ifdef WNT
		if(string_isNullOrSpace(ip))
		{
			ip = REG_get(REG_DCE_KEY, REG_REMOTE_IP);
		}
#endif
		if(ip != NULL)
			remoteIP = TC_DUP(ip);
		initialized = TRUE;
	}
	return remoteIP;
}
int evaluate_internal(const char *action, tag_info_ro dsInfo, tag_info_ro revInfo, kvl_ro additionalProps, obligation_list *obligations, int *nobligation)
{
	int retVal = EVAL_DEFAULT;
	
	char *userName, *userId;
	kvl_p srcAttributes = NULL;
	kvl_p usrAttributes = NULL;
	struct eval_app_s app;
	struct eval_user_s user;
	struct eval_res_s src;
	char resourceName[255];
	
	DBGLOG("Evaluating(%s) for "TAG_INFO_FMT"(ThreadId=%lu)",
		action, TAG_INFO_ARGS(dsInfo), GetCurrentThreadId());

	//initialize out values
	if(obligations != NULL) *obligations = NULL;
	if(nobligation != NULL) *nobligation = 0;

	DBGLOG("Building App Infos...");
	{
		app.name = EVAL_APP_NAME;
		app.url = NULL;
		app.attributes = NULL;
	}
	DBGLOG("Getting User Infos..." );
	{
		tag_t userTag;
		ITK_CALL(POM_get_user(&userName, &userTag));
		NEED_FREE(userName);
		ITK_CALL(POM_get_user_id(&userId));
		NEED_FREE(userId);
		
		DBGLOG("==>Current User(%d) - %s(ID:%s)", userTag, userName, userId);
		user.name = userName;
		user.id = userId;
		usrAttributes = kvl_new(2);
		kvl_add(usrAttributes, "user_id", userId);
		kvl_add(usrAttributes, "tc_username", userName);
		user.attributes = usrAttributes;
	}

	DBGLOG("Constructing Resource Name...");
	{
		int nodeType;
		char *nodeName;

		ITK_CALL(POM_ask_server_info(&nodeType, &nodeName));
		NEED_FREE(nodeName);
		//TODO:build source path
		if(revInfo != NULL)
		{
			sprintf_s(resourceName, sizeof(resourceName), "teamcenter://%s/%s/%s/%s/%s", nodeName, revInfo->className, revInfo->name, dsInfo->className, dsInfo->name);
		}
		else
		{
			sprintf_s(resourceName, sizeof(resourceName), "teamcenter://%s/%s/%s", nodeName, dsInfo->className, dsInfo->name);
		}
		DBGLOG("==>Resource Name:%s", resourceName);
		TC_FREE(nodeName);
		src.name = resourceName;
		src.type = RES_TYPE_PORTAL;
		DBGLOG("==>Generating Source Attribute from Dataset...");
		{
			int i = 0;
			int nattrs = 0;
			char **attrKeys = NULL;
			if (ITK_EVAL(PREF_ask_char_values(PREF_EVALUATION_ATTRS, &nattrs, &attrKeys))
				&& nattrs > 0)
			{
				NEED_FREE(attrKeys);
				srcAttributes = kvl_new(nattrs);
				for (i = 0; i < nattrs; i++)
				{
					char *propValue = NULL;
					if (tag_info_ask_prop_value(dsInfo, attrKeys[i], &propValue))
					{
						NEED_FREE(propValue);
						DBGLOG(PREF_EVALUATION_ATTRS"[%d/%d]='%s':"TAG_INFO_FMT"='%s'",
							i + 1, nattrs, attrKeys[i], TAG_INFO_ARGS(dsInfo), propValue);
					}
					if (string_isNullOrSpace(propValue)
						&& revInfo != NULL)
					{
						TC_FREE(propValue);
						if (tag_info_ask_prop_value(revInfo, attrKeys[i], &propValue))
						{
							NEED_FREE(propValue);
							DBGLOG(PREF_EVALUATION_ATTRS"[%d/%d]='%s':"TAG_INFO_FMT"='%s'",
								i + 1, nattrs, attrKeys[i], TAG_INFO_ARGS(revInfo), propValue);
						}
					}
					if (propValue != NULL)
					{
						kvl_add(srcAttributes, attrKeys[i], propValue);
						TC_FREE(propValue);
					}
					else
					{
						DBGLOG(PREF_EVALUATION_ATTRS"[%d/%d]='%s':Value Not Found", i + 1, nattrs, attrKeys[i]);
					}
				}
				TC_FREE(attrKeys);
			}
			else
			{
				hashtable_ro dsProps = tag_info_get_props(dsInfo);
				hashtable_ro revProps = tag_info_get_props(revInfo);
				srcAttributes = kvl_new(dsProps->count);
				for (i = 0; i < dsProps->count; i++)
				{
					kvl_add(srcAttributes, dsProps->kvps[i].key, (const char *)(dsProps->kvps[i].value));
				}
				if (revProps != NULL)
				{
					for (i = 0; i < revProps->count; i++)
					{
						struct kvp_s kvp = revProps->kvps[i];
						if (!ht_contains_key(dsProps, kvp.key))
						{
							kvl_add(srcAttributes, kvp.key, (const char *)(kvp.value));
						}
					}
				}
			}
			if (additionalProps != NULL
				&& additionalProps->count > 0)
			{
				for (i = 0; i < additionalProps->count; i++)
				{
					DBGLOG("==>AdditionalAttributes[%d/%d]='%s':'%s'", i + 1, additionalProps->count, additionalProps->keys[i], additionalProps->vals[i]);
					kvl_add(srcAttributes, additionalProps->keys[i], additionalProps->vals[i]);
				}
			}
			src.attributes = srcAttributes;
		}
	}
	{
		pep_agent_properties_t properties = { NULL };
		//
		properties.pdpHost = TC_DUP(pref_get_char("NXL_PDPHOST_SERVERAPP", ""));
		properties.pdpPort = TC_DUP(pref_get_char("NXL_PDPPORT_SERVERAPP", "57070"));
		properties.pdpHttps = TC_DUP(pref_get_char("NXL_PDP_HTTPS_SERVERAPP", "false"));
		properties.pdpIgnoreHttps = TC_DUP(pref_get_char("NXL_PDP_IGNOREHTTPS_SERVERAPP", "true"));

		properties.oauth2Host = TC_DUP(pref_get_char("NXL_OAUTH2HOST", ""));
		properties.oauth2Port = TC_DUP(pref_get_char("NXL_OAUTH2PORT", "443"));
		properties.oauth2Https = TC_DUP(pref_get_char("NXL_OAUTH2_HTTPS", "true"));
		properties.oauth2ClientId = TC_DUP(pref_get_char("NXL_OAUTH2_CLIENTID", ""));
		properties.oauth2ClientSecret = TC_DUP(pref_get_char("NXL_OAUTH2_CLIENTSECRET", ""));
		properties.oauth2ClientSecretEncrypted = TRUE;
		retVal = openaz_evaluate(&properties, action, &app, &user, &src, obligations, nobligation);
		TC_FREE2((char*)properties.pdpHost);
		TC_FREE2((char*)properties.pdpPort);
		TC_FREE2((char*)properties.pdpHttps);
		TC_FREE2((char*)properties.pdpIgnoreHttps);
		TC_FREE2((char*)properties.oauth2Host);
		TC_FREE2((char*)properties.oauth2Port);
		TC_FREE2((char*)properties.oauth2Https);
		TC_FREE2((char*)properties.oauth2ClientId);
		TC_FREE2((char*)properties.oauth2ClientSecret);
	}
	if(nobligation != NULL)
		DBGLOG("Got %d obligation objects from Evaluation", *nobligation);

//CLEANUP:
	kvl_free(srcAttributes);
	kvl_free(usrAttributes);
	TC_FREE(userName);
	TC_FREE(userId);
	DBGLOG("Finishing(%d)...", retVal);
	return retVal;
}

//<tagList> must be allocated before this call
int generate_tagging_keys(obligation_list_ro obs, const int obCnt, hashtable_ro allProps, kvl_p tagList)
{
	static char *sysCols[]={"object_type","items_tag","owning_user","object_name","ip_classification","owning_group"
							,"last_mod_user","license_list","gov_classification"};
	static int sysCnt = sizeof(sysCols)/sizeof(sysCols[0]);
	hashtable_p propVals = ht_create(obCnt);//initial the size as obligation count
	int iob;
	int validObs = 0;

	DBGLOG("Checking Obligation Tagging Columns/Values...");
	for(iob = 0; iob < obCnt; iob++)
	{
		int hasAll = FALSE;
		int hasSystem = FALSE;
		DBGLOG("Obligation[%d/%d]%s:%s", iob+1, obCnt, obs[iob].name, obs[iob].policy);
		if(strcmp(obs[iob].name, DRM_OB_NAME)==0)
		{
			const char *taggingType = ht_get_chars(obs[iob].attributes, TAGGING_KEY_TYPE);
			const char *name = ht_get_chars(obs[iob].attributes, TAGGING_KEY_COLUMN_NAME);
			const char *val = ht_get_chars(obs[iob].attributes, TAGGING_KEY_COLUMN_VALUE);
			DBGLOG("==>Tagging='%s' Name='%s' Value='%s'", taggingType, name, val);
			if(strcmp(taggingType, TAGGING_TYPE_USER_DEFINED)==0)
			{
				if(!string_isNullOrSpace(name) 
					&& !string_isNullOrSpace(val))
				{
					//User-Defined colunm
					kvl_add(tagList, name, val);
				}
			}
			else if(strcmp(taggingType, TAGGING_TYPE_SPECIFIC)==0)
			{
				if(!hasAll
					&& !string_isNullOrSpace(name))
				{
					if(ht_tryget_chars(allProps, name, &val))
					{
						ht_set_chars(propVals, name, val);
					}
					DBGLOG("--->Value='%s'", val);
				}
			}
			else if(strcmp(taggingType, TAGGING_TYPE_SYSTEM_COLUMNS)==0)
			{
				if(!hasAll && !hasSystem)
				{
					int isys;
					DBGLOG("System-Columns: Count(%d), Current Size(%d)", sysCnt, propVals->count);
					for(isys=0; isys<sysCnt; isys++)
					{
						if(ht_tryget_chars(allProps, sysCols[isys], &val))
						{
							ht_set_chars(propVals, sysCols[isys], val);
							DBGLOG("==>%s[%d/%d](%s): Added(#%d)-'%s'", taggingType
								, isys+1, sysCnt
								, sysCols[isys], propVals->count, val);
						}
						else
						{
							DBGLOG("==>System-Column[%d/%d](%s): Cannot Find value"
								, isys+1, sysCnt
								, sysCols[isys]);
						}
					}
					hasSystem = TRUE;
				}
			}
			else if(strcmp(taggingType, TAGGING_TYPE_ALL_COLUMNS)==0)
			{
				if(!hasAll)
				{
					DBGLOG("All-Columns: Count(%d), Current Size(%d)", allProps->count, propVals->count);
					ht_merge_chars(propVals, allProps, ondup_override);
					hasAll = TRUE;
				}
			}
			else if(strcmp(taggingType, TAGGING_TYPE_AS_PREFERENCE)==0)
			{
				if(!hasAll)
				{
					int iPrefKey;
					kvl_p prefKeys = pref_get_protect_attributes();
					DBGLOG("Preference-Columns: Count(%d), Current Size(%d)", prefKeys->count, propVals->count);
					for(iPrefKey=0; iPrefKey < prefKeys->count; iPrefKey++)
					{
						if(ht_tryget_chars(allProps, prefKeys->keys[iPrefKey], &val)
							&& !string_isNullOrSpace(val))
						{
							ht_set_chars(propVals, prefKeys->keys[iPrefKey], val);
							DBGLOG("==>%s[%d/%d](%s): Added(#%d)-'%s'", taggingType
								, iPrefKey+1, prefKeys->count
								, prefKeys->keys[iPrefKey], propVals->count, val);
						}
						else if(!string_isNullOrSpace(prefKeys->vals[iPrefKey]))
						{
							ht_set_chars(propVals, prefKeys->keys[iPrefKey], prefKeys->vals[iPrefKey]);
							DBGLOG("==>%s[%d/%d](%s): Cannot Find value, use default('%s')", taggingType
								, iPrefKey+1, prefKeys->count
								, prefKeys->keys[iPrefKey]
								, prefKeys->vals[iPrefKey]);
						}
					}
					kvl_free(prefKeys);
				}
			}
			else
			{
				DBGLOG("==>!!!Invalid Tagging Type(%d)", taggingType);
				continue;
			}
			validObs++;
		}
	}
	DBGLOG("Merging %d System Properties into %d User-Defined columns...", propVals->count, tagList->count);
	if(propVals->count > 0)
	{
		int i=0;
		for(i=0; i<propVals->count; i++)
		{
			kvl_add(tagList, propVals->kvps[i].key, (const char *)(propVals->kvps[i].value));
		}
	}
	ht_free(propVals);
	return validObs;
}

int evaluate_download(tag_info_ro dsInfo, tag_info_ro revInfo)
{
	obligation_list obligations = NULL;
	int nob;
	int ret  = SKIP_TRANSLATION;
	int evalRet;
	PRINT_TAG_INFO(dsInfo);
	PRINT_TAG_INFO(revInfo);
	if((evalRet = evaluate_internal(DOWNLOAD_OPERATION, dsInfo, revInfo, NULL, &obligations, &nob)) != EVAL_DEFAULT
		&& nob > 0)
	{
		int iob;
		DBGLOG(" Got %d obligation objects from Evaluation(EvalReturn=%d)", nob, evalRet);
		for(iob=0; iob<nob; iob++)
		{
			DBGLOG("Obligation[%d/%d]-%s(%s)", iob+1, nob, obligations[iob].name, obligations[iob].policy);
			if(strcmp(obligations[iob].name, DRM_OB_NAME)==0)
			{
				ret = DO_TRANSLATION;
				break;
			}
		}
		obligations_free(obligations, nob);
	}
	else if(evalRet == EVAL_DEFAULT)
	{
		DBGLOG("Evaluation failed, do translation by default");
		ret = DO_TRANSLATION;
	}
	return ret;
}

int evaluate_workflow_protect(tag_info_ro dsInfo, tag_info_ro revInfo, kvl_p tagMap)
{
	obligation_list obligations = NULL;
	int nob;
	
	if(tagMap != NULL
		&& evaluate_internal(DRM_EVAL_OPERATION, dsInfo, revInfo, NULL, &obligations, &nob)!=EVAL_DEFAULT
		&& nob > 0)
	{
		int validObs = 0;
		DBGLOG("Got %d obligation objects from Evaluation", nob);
		if(pref_send_props_for_protect())
		{
			CALL_DBG(validObs = generate_tagging_keys(obligations, nob, tag_info_get_props(dsInfo), tagMap));
			DBGLOG("Found %d Tags from %d valid obligations", tagMap->count, validObs);
		}
		else
		{
			//the property will be generated by dispatcher client, we only check whether the specific obligation is there
			int iob;
			for(iob = 0; iob < nob; iob++)
			{
				if(strcmp(obligations[iob].name, DRM_OB_NAME)==0)
				{
					const char *taggingType = ht_get_chars(obligations[iob].attributes, TAGGING_KEY_TYPE);
					const char *name = ht_get_chars(obligations[iob].attributes, TAGGING_KEY_COLUMN_NAME);
					const char *val = ht_get_chars(obligations[iob].attributes, TAGGING_KEY_COLUMN_VALUE);
					DBGLOG("Obligation[%d/%d]%s:%s(Tagging='%s' Name='%s' Value='%s')", iob+1, nob, obligations[iob].name, obligations[iob].policy, taggingType, name, val);
					validObs++;
				}
				else
				{
					DBGLOG("Obligation[%d/%d]%s:%s", iob+1, nob, obligations[iob].name, obligations[iob].policy);
				}
			}
		}
		//free obligations
		obligations_free(obligations, nob);
		if(validObs > 0)
			return DO_TRANSLATION;
	}
	return SKIP_TRANSLATION;
}

int evaluate_workflow_unprotect(const char *wfName, tag_info_ro dsInfo, tag_info_ro revInfo)
{
	int ret = SKIP_TRANSLATION;
	obligation_list obligations = NULL;
	int nob = 0;
	kvl_p additionalProps = kvl_new(2);
	kvl_add(additionalProps, "operation-source", "workflow");
	DBGLOG("Evaluating for unprotect in workflow-'%s'", wfName);
	if(wfName != NULL)
	{
		kvl_add(additionalProps, "workflow", wfName);
	}
	if(evaluate_internal(DCE_OPERATION_UNPROTECT, dsInfo, revInfo, additionalProps, &obligations, &nob) != EVAL_DENY
		&& nob > 0)
	{
		//send unprotect translation when got deny result or eval failed
		//some obligation must be present to indicate that some policy is matched
		ret = DO_TRANSLATION;
	}
	//free obligations
	obligations_free(obligations, nob);
	kvl_free(additionalProps);
	return ret;
}