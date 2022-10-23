//
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
//
#include <utils_log.h>
#include <nxl_evaluator_remote.h>
#include <utils_windows.h>
#include <utils_mem.h>
#include <config/metadata_priority.h>

//link node for metadata priority
typedef struct _metadta_priority_s
{
	char *key;
	string_list_p values;
	struct _metadta_priority_s *next;
}metadata_priority_t, *metadata_priority_p;

static metadata_priority_p s_priorities = NULL;

//cache metadata priorities in memory and registery
void metadata_priorities_cache(metadata_priority_p priorities)
{
	//free the old cache
	while(s_priorities != NULL)
	{
		metadata_priority_p next = s_priorities->next;
		//free member
		string_free(s_priorities->key);
		string_list_free(s_priorities->values);
		//free container
		NXL_FREE(s_priorities);
		//go next
		s_priorities = next;
	}
	//cache in memory
	s_priorities = priorities;
}

#define REG_KEY_PRIORITIES "SOFTWARE\\NextLabs\\TeamcenterRMX\\NXIntegration\\MetadataPriorities"
void metadata_priorities_save_to_registry(metadata_priority_p priorities)
{
	//delete old registry values
	REG_CALL(RegDeleteKey(HKEY_LOCAL_MACHINE, REG_KEY_PRIORITIES));
	if(priorities != NULL)
	{
		HKEY prioritiesKey;
		//create new key
		if (REG_CALL(RegCreateKey(HKEY_LOCAL_MACHINE, REG_KEY_PRIORITIES, &prioritiesKey)))
		{
			//file the registry key with the priorities
			while(priorities != NULL)
			{
				char *valName = priorities->key;
				//pack priority values into REG_MULTI_SZ which is in following format
				//str1\0str2\0str3\0\0
				DWORD bufSize = 0x100;
				char *buffer = NULL;
				DWORD ival = 0;
				DWORD bufLen = 0;
				DWORD nValues = 0;
				NXL_ALLOCN(buffer, char, bufSize);
				for(ival = 0; ival < priorities->values->count; ival++)
				{
					DWORD valLen = strlen(priorities->values->items[ival]);
					while((bufLen + valLen + 1 + 1) > bufSize)
					{
						bufSize *= 2;
						NXL_REALLOC(buffer, char, bufSize);
						DBGLOG("Reallocating buffer to %lu", bufSize);
					}
					//append to tail
					strcpy(buffer + bufLen, priorities->values->items[ival]);
					//move ibuffer
					nValues++;
					bufLen += valLen + 1;
				}
				//package string is terminated with two \0
				buffer[bufLen++] = '\0';
				DBGLOG("ValName='%s' Buffer:Size=%lu Len=%lu nValues=%lu ExpectedValues=%u", valName, bufSize, bufLen, nValues, priorities->values->count);
				//set value
				if(REG_CALL(RegSetValueEx(prioritiesKey, valName, NULL, REG_MULTI_SZ, (PBYTE)buffer, bufLen)))
				{
					DBGLOG("ValueAdded:'%s'='%s'", valName, buffer);
				}
				NXL_FREE(buffer);
				priorities = priorities->next;
			}
			RegCloseKey(prioritiesKey);
		}
	}
}

/*
	Load priority definition from registry in following structure
	TeamcenterRMX
		NXIntegration
			 MetadataPriorities
				dummyKey(REG_MULTI_SZ):dummyValue1 dummyValue2 dummyval3
				ip_classification(REG_MULTI_SZ):top-secret super-secret secret
	the export reg file will looks like following
	[HKEY_LOCAL_MACHINE\Software\NextLabs\TeamcenterRMX\NXIntegration\MetadataPriorities]
	"dummyKey"=hex(7):64,00,75,00,6d,00,6d,00,79,00,56,00,61,00,6c,00,75,00,65,00,\
	  31,00,00,00,64,00,75,00,6d,00,6d,00,79,00,56,00,61,00,6c,00,75,00,65,00,32,\
	  00,00,00,64,00,75,00,6d,00,6d,00,79,00,56,00,61,00,6c,00,75,00,65,00,33,00,\
	  00,00
	"ip_classification"=hex(7):74,00,6f,00,70,00,2d,00,73,00,65,00,63,00,72,00,65,\
	  00,74,00,00,00,73,00,75,00,70,00,65,00,72,00,2d,00,73,00,65,00,63,00,72,00,\
	  65,00,74,00,00,00,73,00,65,00,63,00,72,00,65,00,74,00,00,00,00,00
*/
static metadata_priority_p metadata_priorities_load_from_registry()
{
	//1, query policy and save to cache and registry
	metadata_priority_t prioritiesRoot = {0};
	metadata_priority_p lastNode = &prioritiesRoot;

	HKEY prioritiesKey;
	//open new key
	if (REG_CALL(RegOpenKey(HKEY_LOCAL_MACHINE, REG_KEY_PRIORITIES, &prioritiesKey)))
	{
		int ivalue = 0;
		LONG returnStatus;
		char nameBuff[16383];
		DWORD nameSize = sizeof(nameBuff);
		DWORD valType;
		LPBYTE valBuff = NULL;
		DWORD valSize = 100;
		DWORD valLen = valSize;
		NXL_ALLOCN(valBuff, BYTE, valSize);
		DBGLOG("Initialized value buffer as %lu", valSize);
		//enumerate values
		while((returnStatus = RegEnumValue(prioritiesKey, ivalue, nameBuff, &nameSize, NULL, &valType, valBuff, &valSize))
			!= ERROR_NO_MORE_ITEMS)
		{
			DBGLOG("[%d]ValueName='%s'(nameLen='%lu') ValueType='%lu' ValueLen='%lu'", ivalue, nameBuff, nameSize, valType, valLen);
			if(returnStatus == ERROR_SUCCESS)
			{
				//read
				if(valType == REG_MULTI_SZ)
				{
					int ichar = 0;
					//TODO:UNICODE?
					const char *packedString = (char*)valBuff;
					//create new node
					metadata_priority_p newNode = NULL;
					NXL_ALLOC(newNode, metadata_priority_t);
					newNode->key = string_dup(nameBuff);
					newNode->values = string_list_new();
					newNode->next = NULL;
					//load values from packed string into newNode->values
					//the packed string is in following format
					//str1\0str2\0str3\0\0
					while(ichar < valLen)
					{
						char const *str = packedString+ichar;
						int len = strlen(str);
						if(len > 0)
						{
							DBGLOG("==>[%d]'%s'", newNode->values->count, str);
							string_list_add(newNode->values, str);
							//move to next sub string
							ichar += len + 1;
						}
						else
						{
							//this is last terminating null
							break;
						}
					}
					//append new node
					lastNode->next = newNode;
					lastNode = newNode;
				}
				//continue reading next value
				ivalue++;
			}
			else if(returnStatus == ERROR_MORE_DATA)
			{
				//allocate 
				DBGLOG("[%d]Requiring more buffer space(ValueLen:%lu ValueSize:%lu)", ivalue, valLen, valSize);
				if(valLen > valSize)
				{
					valSize = valLen;
					NXL_REALLOC(valBuff, BYTE, valSize);
				}
				else
				{
					//TODO:something wrong here
					//continue reading next value
					ivalue++;
				}
			}
			else
			{
				report_win_error(returnStatus, ERROR_SUCCESS, __FUNCTION__, __FILE__, __LINE__, "RegEnumValue");
				break;
			}
			//reset the buffer len for next query
			valLen = valSize;
			nameSize = sizeof(nameBuff);
		}
		NXL_FREE(valBuff);
		RegCloseKey(prioritiesKey);
	}
	return prioritiesRoot.next;
}

#define POLICY_EVAL_ACTION "TCDRM_CONFIGURATION"
#define POLICY_OBLIGATION_NAME "TCDRM_TAGCONFIG"
#define TAGCONFIG_KEY "TagKey"
#define TAGCONFIG_VALUES "ValueList"
//read metadata priorities by policy evaluation
static BOOL metadata_priorities_load_from_policy(metadata_priority_p *priorities)
{
	eval_result_t evalResult = EVAL_DEFAULT;
	struct eval_app_s app = {0};
	eval_user_ro pUser = default_eval_user();
	struct eval_res_s res = {0};
	//init app
	app.name = PROJECT_NAME;
	//NOTE:resource information is not necessary for this evaluation
	//but java sdk requires some non-empty values for sending evaluation
	res.name = TARGETFILENAME;
	res.type = RES_TYPE_PORTAL;
	//
	obligation_list obs = NULL;
	int nob = 0;
	*priorities = NULL;

	if((evalResult = remote_evaluate(NULL, POLICY_EVAL_ACTION, &app, pUser, &res, &obs, &nob))
		!= EVAL_DEFAULT)
	{
		//1, query policy and save to cache and registry
		metadata_priority_t prioritiesRoot = {0};
		metadata_priority_p lastNode = &prioritiesRoot;
		//TBD:searching for allow/deny ?
		if(nob > 0)
		{
			int iob;
			for(iob = 0; iob < nob; iob++)
			{
				if(stricmp(obs[iob].name, POLICY_OBLIGATION_NAME) == 0)
				{
					const char *keyName;
					if(ht_tryget_chars(obs[iob].attributes, TAGCONFIG_KEY, &keyName))
					{
						const char *valList;
						if(ht_tryget_chars(obs[iob].attributes, TAGCONFIG_VALUES, &valList))
						{
							//create new node
							metadata_priority_p newNode = NULL;
							//initialize the node
							NXL_ALLOC(newNode, metadata_priority_t);
							newNode->key = string_dup(keyName);
							newNode->values = string_list_new();
							newNode->next = NULL;
							//fill value list
							string_split(valList, ";", newNode->values);
							DBGLOG("[%d/%d]Name='%s' " TAGCONFIG_KEY "='%s' " TAGCONFIG_VALUES "='%s' nValues=%d"
								, iob+1, nob, obs[iob].name, keyName, valList, newNode->values->count);
							//
							lastNode->next = newNode;
							lastNode = newNode;
						}
						else
						{
							DBGLOG("[%d/%d]Name='%s' " TAGCONFIG_KEY "='%s':Failed to obtain-" TAGCONFIG_VALUES, iob+1, nob, obs[iob].name, keyName);
						}
					}
					else
					{
						DBGLOG("[%d/%d]Name='%s':Failed to obtain-" TAGCONFIG_KEY, iob+1, nob, obs[iob].name);
					}
				}
				else
				{
					DBGLOG("[%d/%d]Name='%s'", iob+1, nob, obs[iob].name);
				}
			}
		}
		else
		{
			DBGLOG("No obligation is returned from evaluation");
		}
		obligations_free(obs, nob);
		*priorities = prioritiesRoot.next;
		return TRUE;
	}
	return FALSE;
}


/*
1, use policy evaluation to get the metadata priorities
	1.1 if success, save the priority in memory and registry(?TBD)
2, if query failed, use the priorities in memory
3, if memory cached priority is null, load priority from registry
*/
void metadata_priorities_load()
{
	metadata_priority_p priorities;	
	if(!metadata_priorities_load_from_policy(&priorities))
	{
		//load configuration from policy failed
		//return the priorities cached in memory if it's not empty
		if(s_priorities == NULL)
		{
			priorities = metadata_priorities_load_from_registry();
			if(priorities != NULL)
			{
				//cache in memory
				metadata_priorities_cache(priorities);
			}
		}
	}
	else
	{
		//cache in memory
		metadata_priorities_cache(priorities);
		//cache in registry
		metadata_priorities_save_to_registry(priorities);
	}
}

#define COMPARE_EQUALS 0
#define COMPARE_GREATERTHAN 1
#define COMPARE_LESSTHAN -1
int metadata_compare(const char *key, const char *leftVal, const char *rightVal, int *cmp)
{
	if(key == NULL || leftVal == NULL || rightVal == NULL)
	{
		//invalid key values
		return FALSE;
	}
	else if(stricmp(leftVal, rightVal) == 0)
	{
		//val1 and val2 are duplicated
		*cmp = COMPARE_EQUALS;
		return TRUE;
	}
	else
	{
		//load priorities from memory
		metadata_priority_p priorityNode = s_priorities;
		while(priorityNode != NULL)
		{
			if(priorityNode->key != NULL
				&& priorityNode->values != NULL
				&& stricmp(priorityNode->key, key) == 0)
			{
				int ival, ileft, iright;
				ileft = iright = -1;
				//searching the index of left and right values
				for(ival = 0; ival < priorityNode->values->count && (ileft < 0 || iright < 0); ival++)
				{
					if(ileft < 0
						&& stricmp(priorityNode->values->items[ival], leftVal) == 0)
					{
						ileft = ival;
					}
					else if(iright < 0
						&& stricmp(priorityNode->values->items[ival], rightVal) == 0)
					{
						iright = ival;
					}
				}
				DBGLOG("Key-'%s':'%s'=%d '%s'=%d", key, leftVal, ileft, rightVal, iright);
				//return compare result only when both values are present in this config
				if(ileft >= 0 && iright >= 0)
				{
					//smaller index means higher priority
					*cmp = ileft > iright ? COMPARE_LESSTHAN : COMPARE_GREATERTHAN;
					return TRUE;
				}
				/*
				//if only one of the values presents in this config, treat the present one has higher priority
				else if(ileft >=0 || iright >= 0)
				{
					//positive index means higher priority
					*cmp = ileft > iright ? COMPARE_GREATERTHAN : COMPARE_LESSTHAN;
					return TRUE;
				}
				*/
			}
			priorityNode = priorityNode->next;
		}
		//the priority order of this key is not defined, return compare unsuccessful
		return FALSE;
	}
}

BOOL metadata_merge(kvl_p target, kvl_ro source)
{
	if(target != NULL
		&& source != NULL)
	{
		int isrc;
		for(isrc = 0; isrc < source->count; isrc++)
		{
			const char *srcKey = source->keys[isrc];
			const char *srcVal = source->vals[isrc];
			if(srcKey != NULL && srcVal != NULL)
			{
				int itar = 0;
				BOOL handled = FALSE;//by default this key value need to be added
				while(itar < target->count)
				{
					if(stricmp(target->keys[itar], srcKey) == 0)
					{
						int cmp;
						//found metadata with same key, try compare
						const char *tarVal = target->vals[itar];
						if(metadata_compare(srcKey, srcVal, tarVal, &cmp))
						{
							//compare success
							if(cmp == 0)
							{
								//duplicated metadata
								DBGLOG("[%d/%d]Duplicated:'%s'='%s'(Target[%d/%d]='%s')", isrc+1, source->count, srcKey, srcVal, itar, target->count, tarVal);
								handled = TRUE;
								break;
							}
							else if(cmp > 0)
							{
								//source value has higher priority than target value
								if(handled)
								{
									//this key value pair has been set before
									DBGLOG("[%d/%d]Removing:'%s'='%s' Target[%d/%d]:'%s'", isrc+1, source->count, srcKey, srcVal, itar, target->count, tarVal);
									kvl_removeAt(target, itar);
									//skip increase itar
									continue;
								}
								else
								{
									//override target Metadata
									DBGLOG("[%d/%d]Overriding:'%s'='%s' Target[%d/%d]:'%s'", isrc+1, source->count, srcKey, srcVal, itar, target->count, tarVal);
									kvl_set(target, itar, srcKey, srcVal);
									handled = TRUE;
								}
							}
							else
							{
								DBGLOG("[%d/%d]Skipped:'%s'='%s' Target[%d/%d]:'%s'", isrc+1, source->count, srcKey, srcVal, itar, target->count, tarVal);
								handled = TRUE;
								break;
							}
						}
						else
						{
							DBGLOG("[%d/%d]CompareFailed:'%s'='%s' Target[%d/%d]:'%s'", isrc+1, source->count, srcKey, srcVal, itar, target->count, tarVal);
						}
					}
					//goto next key value pair in target
					itar++;
				}
				if(!handled)
				{
					//add this key to target kvl when
					//1, this key is not present in the target kvl
					//2, the priority of this value is not defined
					kvl_add(target, srcKey, srcVal);
					DBGLOG("[%d/%d]Added:'%s'='%s'", isrc+1, source->count, srcKey, srcVal);
				}
			}
			else
			{
				DBGLOG("[%d/%d]Invalid:'%s'='%s'", isrc+1, source->count, srcKey, srcVal);
			}
		}
		return TRUE;
	}
	return FALSE;
}