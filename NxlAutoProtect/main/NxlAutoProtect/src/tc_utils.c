#include <stdlib.h>
//ITK
#include <tccore/aom_prop.h>
#include <tccore/aom.h>
#include <tccore/item.h>
#include <tccore/grm.h>
#include <tc/preferences.h>
#include <constants/constants.h>
#include <ae/dataset.h>
#include <ae/dataset_msg.h>
#include <sa/tcfile.h>
#include <res/res_itk.h>
#include <pom/enq/enq.h>
#include <common/tc_ver_info.h>
//NextLabs
#include "dce_log.h"
#include "tc_utils.h"
#include "dce_mem.h"
#include "utils_string.h"
#include <utils_system.h>
#include <key_value_list.h>
#include <FSCNativeClientProxy.h>

#include <nxl_action_handler.h>

const char *get_class_name(obj_type type)
{
	const char *typeName = NULL;
	switch(type)
	{
		case type_item:
			typeName = ITEM_item_class_name_c;
			break;
		case type_item_revision:
			typeName = ITEM_rev_class_name_c;
			break;
		case type_dataset:
			typeName = "Dataset";
			break;
		case type_file:
			typeName = "ImanFile";
			break;
		default:
			break;
	}
	return typeName;
}

tag_t get_class_id(obj_type type)
{
	static tag_t cids[type_unknown];
	tag_t cid = cids[type];
	if(cid == NULLTAG)
	{
		const char *typeName = get_class_name(type);
		if(typeName != NULL
			&& ITK_EVAL(POM_class_id_of_class(typeName, &cid)))
		{
			DBGLOG("Class Tag of '%s'(Type=%d) is initialized as '%d'", typeName, type, cid);
			cids[type] = cid;
		}
		else
		{
			return NULLTAG;
		}
	}
	//DTLLOG("cids[%d]=%d", type, cid);
	return cid;
}

int protection_mark;
int set_protection_mark()
{
	EMH_set_protect_mark(&protection_mark);
	DBGLOG("Set protect mark=%d", protection_mark);
	return protection_mark;
}
void clear_protection_mark()
{
	DBGLOG("Clearing protect mark(%d)...", protection_mark);
	EMH_clear_protect_mark(protection_mark);
	protection_mark = 0;
}
logical report_error(const char *file, int line, const char *func, const char *call, int ret, logical exit_on_error)
{
    if (ret != ITK_ok)
    {
		char *errMsg = NULL;
		if(EMH_ask_error_text (ret, &errMsg)==ITK_ok)
		{
			NEED_FREE(errMsg);
			DBGLOG("%s(): %s returns %d(ErrorText='%s') @Line-%d in File-%s", func, call, ret, errMsg, line, file);
			TC_FREE(errMsg);
		}
		else
		{
			DBGLOG("%s(): %s returns %d @Line-%d in File-%s", func, call, ret, line, file);
		}
		error_list();
		//clear all errors
		if((line = EMH_clear_errors()) != ITK_ok)
		{
			DBGLOG("EMH_clear_errors() returns %d", line);
		}

        if (exit_on_error)
        {
			SYSLOG("%s(): Exiting Program(%d)...", func, ret);
            exit (ret);
        }
		return false;
    }
	else
	{
		//DTLLOG("%s():%s returns %d", func, call, ret);
	}
    return true;
}
void error_list()
{
	int
		n_errors = 0;
	const int *severities = NULL,
		*statuses = NULL;
	const char **messages;
	int ret = EMH_ask_errors(&n_errors, &severities, &statuses, &messages);
	if(ITK_ok == ret)
	{
		int ierror = 0;
		for(ierror=0; ierror<n_errors; ierror++)
		{
			DBGLOG("Error[%d/%d]Severity=%d Status=%d Message='%s'", ierror+1, n_errors, severities[ierror], statuses[ierror], messages[ierror]);
		}
	}
	else
	{
		ERRLOG("EMH_ask_errors returns %d", ret);
	}
}

typedef struct PROP_INFO
{
	char *propName;
	PROP_type_t propType;
	char *propTypeStr;

	PROP_value_type_t valueType;
	char *valueTypeStr;
	union {
		double dblVal;
		float flVal;
		int intVal;
		short shortVal;
		long longVal;
		char charVal;
		char *strVal;
		logical blVal;
		date_t dtVal;
	} propValue;
	char **displayValues;
	int cntValues;
} PPInfo;

void free_prop_info(PPInfo *propInfo)
{
	TC_FREE(propInfo->propName);
	TC_FREE(propInfo->propTypeStr);
	TC_FREE(propInfo->valueTypeStr);
	TC_FREE(propInfo->displayValues);
	TC_FREE(propInfo);
}


PPInfo *get_property_info(tag_t tagdata, char * propName)
{
	PPInfo *propInfo;
	int iValue;
	TC_ALLOC(propInfo, PPInfo);
	DTLLOG("Checking Property(%d):%s", tagdata, propName);

	propInfo->propName = TC_DUP(propName);
	DTLLOG("Property Name(%d):%s", tagdata, propInfo->propName);

	ITK_CALL(AOM_ask_property_type(tagdata, propName, &propInfo->propType, &propInfo->propTypeStr));
	NEED_FREE(propInfo->propTypeStr);
	DTLLOG("Property Type(%d):%s(%d)", tagdata, propInfo->propTypeStr, propInfo->propType);

	ITK_CALL(AOM_ask_value_type(tagdata, propName, &propInfo->valueType, &propInfo->valueTypeStr));
	NEED_FREE(propInfo->valueTypeStr);
	DTLLOG("Value Type(%d):%s(%d)", tagdata, propInfo->valueTypeStr, propInfo->valueType);

	ITK_CALL(AOM_ask_displayable_values(tagdata, propName, &propInfo->cntValues, &propInfo->displayValues));
	NEED_FREE(propInfo->displayValues);
	DTLLOG("Values Count(%d):%d", tagdata, propInfo->cntValues);

#ifdef DEBUG
	for(iValue = 0; iValue < propInfo->cntValues; iValue++)
	{
		DTLLOG("==>Value-%d:'%s'", iValue, propInfo->displayValues[iValue]);
	}
#endif
	return propInfo;
}
/*
	implementation of tag_info_p 
*/
typedef struct tag_info_internal_s
{
	hashtable_p props;
	tag_info_p parent;
}*tag_info_internal_p;
#define PROPS(ti) ((tag_info_internal_p)ti->reserved)->props
#define PARENT(ti) ((tag_info_internal_p)ti->reserved)->parent
void tag_info_free(tag_info_p tagInfo)
{
	if(tagInfo != NULL)
	{
		hashtable_p props = PROPS(tagInfo);
		tag_info_p parent = PARENT(tagInfo);
		
		//tag_info_debug_lock(tagInfo);
		TC_FREE(tagInfo->className);
		TC_FREE(tagInfo->name);
		TC_FREE(tagInfo->reserved);
		ht_free(props);
		tag_info_free(parent);
	}
	TC_FREE(tagInfo);
}
tag_info_p tag_info_new(const tag_t tagdata)
{
	//initialize
	tag_info_p tagInfo;
	TC_ALLOC(tagInfo, struct tag_info_s);
	tagInfo->tagId = tagdata;
	tagInfo->rootType = type_unknown;
	tagInfo->name = NULL;
	tagInfo->className = NULL;
	TC_ALLOC(tagInfo->reserved, struct tag_info_internal_s);
	PROPS(tagInfo) = NULL;
	PARENT(tagInfo) = NULL;
	//get class id
	if(ITK_EVAL(POM_class_of_instance(tagdata, &tagInfo->classId)))
	{
		int i;
		//get class name
		ITK_CALL(POM_name_of_class(tagInfo->classId, &tagInfo->className));
		NEED_FREE(tagInfo->className);
		//check if the attachment can be the descendant of ItemRevision
		tagInfo->rootType = type_unknown;
		for(i = 0; i < type_unknown; i++)
		{
			logical isDescendant;
			tag_t cid;
			if((cid = get_class_id((obj_type)i)) != NULLTAG
				&&ITK_EVAL(POM_is_descendant(cid, tagInfo->classId, &isDescendant))
				&&isDescendant)
			{
				tagInfo->rootType = (obj_type)i;
				break;
			}
		}
		ITK_CALL(AOM_ask_name(tagdata, &tagInfo->name));
		NEED_FREE(tagInfo->name);
	}
	//tag_info_debug_lock(tagInfo);
	return tagInfo;
}
void tag_info_debug_lock(tag_info_ro tagInfo)
{
	if(tagInfo != NULL)
	{
		logical bl;
		int lockToken;
		if(ITK_EVAL(RES_is_checked_out(tagInfo->tagId, &bl)))
		{
			DBGLOG(TAG_INFO_FMT"IsCheckout=%d", TAG_INFO_ARGS(tagInfo), bl);
		}
		if(ITK_EVAL(POM_modifiable(tagInfo->tagId, &bl)))
		{
			DBGLOG(TAG_INFO_FMT"modifiable=%d", TAG_INFO_ARGS(tagInfo), bl);
		}
		if(ITK_EVAL(POM_is_loaded(tagInfo->tagId, &bl)))
		{
			DBGLOG(TAG_INFO_FMT"IsLoaded=%d", TAG_INFO_ARGS(tagInfo), bl);
		}
		if(ITK_EVAL(POM_is_cached(tagInfo->tagId, &bl)))
		{
			DBGLOG(TAG_INFO_FMT"IsCached=%d", TAG_INFO_ARGS(tagInfo), bl);
		}
		if(ITK_EVAL(POM_ask_instance_lock(tagInfo->tagId, &lockToken)))
		{
			if(lockToken != POM_no_lock)
			{
				char *userName, *nodeName;
				date_t date;
				if(ITK_EVAL(POM_who_locked_instance(tagInfo->tagId, &userName, &date, &nodeName)))
				{
					DBGLOG(TAG_INFO_FMT"Lock=%d(%s) LockBy=%s NodeName=%s", TAG_INFO_ARGS(tagInfo), lockToken, (lockToken==POM_read_lock?"POM_read_lock":"POM_modify_lock"), userName, nodeName);
					TC_FREE(userName);
					TC_FREE(nodeName);
				}
			}
			else
			{
				DBGLOG(TAG_INFO_FMT"Lock=%d(POM_no_lock)", TAG_INFO_ARGS(tagInfo), lockToken);
			}
		}
	}
}

int get_specific_properties_values(tag_t tagData, char *listKeys[], int cntKeys, char *listValues[])
{
	int index, cntFound,cntValues;
	char **values;
	tag_info_p tagInfo = tag_info_new(tagData);

	for(index = 0,cntFound = 0; index < cntKeys; index++)
	{
		if(ITK_EVAL(AOM_ask_displayable_values(tagData, listKeys[index], &cntValues, &values)))
		{
			NEED_FREE(values);
			listValues[index] = TC_DUP(strings_join(values, cntValues, ";"));
			cntFound++;
			DBGLOG(TAG_INFO_FMT"[%d/%d]'%s'='%s'", TAG_INFO_ARGS(tagInfo), index + 1, cntKeys, listKeys[index], listValues[index]);
			TC_FREE(values);
		}
		else
		{
			listValues[index] = NULL;
		}
	}
	tag_info_free(tagInfo);

	return cntFound;
}
BOOL imf_delete_file(tag_t toBeDeletedFile)
{
	BOOL deleted = FALSE;
	if (toBeDeletedFile != NULLTAG)
	{
		char *path = NULL;
		DBGLOG("Deleting: %u", toBeDeletedFile);
		ITK_EVAL(AOM_lock_for_delete(toBeDeletedFile));
		ITK_EVAL(IMF_ask_file_pathname2(toBeDeletedFile, SS_MACHINE_TYPE, &path));
		NEED_FREE(path);
		if (ITK_EVAL(AOM_delete(toBeDeletedFile)))
		{
			DBGLOG("Deleted: %s", path);
			deleted = TRUE;
		}
		else
		{
			DBGLOG("DeletionFailed: %s", path);
			ITK_EVAL(AOM_unlock(toBeDeletedFile));
		}
		TC_FREE(path);
	}
	return deleted;
}
#define QUERY_NAME "nxl::search_datatset_for_file1"
#define QUERY_ARG "filetag1"
#define QUERY_WHERE "where1"
tag_t imanfile_search_dataset(tag_info_ro fileInfo)
{
	static tag_t initialised = NULLTAG;
	int n_cols=0,n_rows=0;
	void *** values = 0;
	tag_t datasetTag = NULLTAG;
	if ( initialised == NULLTAG )
	{
		const char * select_attr_list[1] = {"puid" };
		const int n_attrs = 1;
		ITK_CALL( POM_enquiry_create ( QUERY_NAME ) );
		ITK_CALL( POM_enquiry_add_select_attrs ( QUERY_NAME, "Dataset", n_attrs, select_attr_list ) );
		ITK_CALL( POM_enquiry_set_tag_value ( QUERY_NAME, QUERY_ARG, 1, &(fileInfo->tagId), POM_enquiry_bind_value ) );
		ITK_CALL( POM_enquiry_set_attr_expr ( QUERY_NAME, QUERY_WHERE, "Dataset", "ref_list", POM_enquiry_in, QUERY_ARG ) );
		ITK_CALL( POM_enquiry_set_where_expr ( QUERY_NAME, QUERY_WHERE ) );
		POM_cache_for_session (&initialised);
		initialised = 1;
	}
	else
	{
		ITK_CALL( POM_enquiry_set_tag_value ( QUERY_NAME, QUERY_ARG, 1, &(fileInfo->tagId), POM_enquiry_bind_value ) );
	}
	ITK_CALL( POM_enquiry_execute ( QUERY_NAME, &n_rows, &n_cols, &values ) );
	NEED_FREE(values);
	DBGLOG("POM_enquiry_execute returns %d rows %d columns", n_rows, n_cols);
	if ( n_rows >= 1 && n_cols >=1)
	{
		int irow;
		for(irow=0; irow < n_rows; irow++)
		{
			tag_t dsTag = *((tag_t *)values[irow][0]);
			DTLLOG("Row[%d/%d]Tag=%u", irow+1, n_rows, dsTag);
			if(ITK_EVAL(AE_ask_dataset(dsTag, &datasetTag))
				&& datasetTag != NULLTAG)
			{
				break;
			}
		}
	}
	TC_FREE( values );
	DBGLOG("Rev0Tag=%u", datasetTag);
	return datasetTag;
}
tag_info_ro tag_info_get_parent(tag_info_ro tagInfo)
{
	if(tagInfo == NULL)
		return NULL;
	if(PARENT(tagInfo) == NULL)
	{
		tag_info_p parentInfo = NULL;
		obj_type parentType = type_unknown;
		//decide parent type
		switch(tagInfo->rootType)
		{
			case type_dataset:
				parentType = type_item_revision;
				break;
			case type_item_revision:
				parentType = type_item;
				break;
			case type_item:
			default:
				break;
		}
		if(parentType != type_unknown)
		{
			int primCnt;
			tag_t *primObjs;
			//the parent is not specified
			if(ITK_EVAL(GRM_list_primary_objects_only(tagInfo->tagId, NULLTAG, &primCnt, &primObjs)))
			{
				NEED_FREE(primObjs);
				if(primCnt > 0)
				{
					int index;
					for(index=0; index < primCnt; index++)
					{
						tag_info_p primInfo = tag_info_new(primObjs[index]);
						if(primInfo->rootType == parentType)
						{
							parentInfo = primInfo;
							DBGLOG(TAG_INFO_FMT":Primary[%d/%d]-"TAG_INFO_FMT" IsParent=TRUE", TAG_INFO_ARGS(tagInfo), index+1, primCnt, TAG_INFO_ARGS(primInfo));
							break;
						}
						DBGLOG(TAG_INFO_FMT":Primary[%d/%d]-"TAG_INFO_FMT" ExpectedType=%d", TAG_INFO_ARGS(tagInfo), index+1, primCnt, TAG_INFO_ARGS(primInfo), parentType);
						CALL_DTL(tag_info_free(primInfo));
					}
				}
				else
				{
					DBGLOG(TAG_INFO_FMT" has 0 primary object", TAG_INFO_ARGS(tagInfo));
				}
				TC_FREE(primObjs);
			}
		}
		PARENT(tagInfo) = parentInfo;
		return parentInfo;
	}
	return PARENT(tagInfo);
}
BOOL is_property_blacklisted(const char *propName)
{
	static hashtable_ro blacklist = NULL;
	BOOL ret = FALSE;
	if(string_isNullOrSpace(propName))
		return TRUE;
	if(blacklist == NULL)
	{		
		//initializing black list
		char **propNames = NULL;
		int nProps = -1;
		hashtable_p ht = ht_create(10);
		if(ITK_EVAL(PREF_ask_char_values(PREF_PROPERTIES_BLACK_LIST, &nProps, &propNames)))
		{
			int itype;
			NEED_FREE(propNames);
			DBGLOG(PREF_PROPERTIES_BLACK_LIST" constains %d values", nProps);
			for(itype=0; itype<nProps; itype++)
			{
				const char *tolower = CharLower(propNames[itype]);
				DBGLOG("==>%s", tolower);
				ht_set_chars(ht, tolower, "TRUE");
			}
			TC_FREE(propNames);
		}
		blacklist = ht;
	}
	if(blacklist != NULL
		&& blacklist->count > 0)
	{
		char *lower = string_toLower(propName);
		ret = ht_contains_key(blacklist, lower);
		string_free(lower);
	}
	return ret;
}
BOOL is_reference_require_auto_deletion(const char *dsType, const char *referenceName)
{
	BOOL required = FALSE;
	string_list_p patterns = pref_nr_auto_delete_on_conflict();
	if (patterns->count > 0) {
		const char *ref = strings_join_v("|", 2, dsType, referenceName);
		int i;
		for (i = 0; i < patterns->count; i++) {
			if (string_wildcmp(ref, patterns->items[i]))
			{
				DBGLOG("'%s' matched with '%s'", ref, patterns->items[i]);
				required = TRUE;
				break;
			}
		}
		DBGLOG("'%s':matched=%d", ref, required);
	}
	string_list_free(patterns);
	return required;

}
BOOL is_reference_require_metadatasync(const char *dsType, const char *referenceName)
{
	BOOL required = FALSE;
	string_list_p patterns = pref_nr_metadatasyn_on_import();
	if (patterns->count > 0) {
		const char *ref = strings_join_v("|", 2, dsType, referenceName);
		int i;
		for (i = 0; i < patterns->count; i++) {
			if (string_wildcmp(ref, patterns->items[i]))
			{
				DBGLOG("'%s' matched with '%s'", ref, patterns->items[i]);
				required = TRUE;
				break;
			}
		}
		DBGLOG("'%s':matched=%d", ref, required);
	}
	string_list_free(patterns);
	return required;

}
hashtable_ro tag_info_get_props(tag_info_ro tagInfo)
{
	if(tagInfo == NULL)
		return NULL;
	if(PROPS(tagInfo) == NULL)
	{
		int allPropCnt,iProp;
		char **allPropKeys;
		hashtable_p props;
		if(!ITK_EVAL(AOM_ask_prop_names(tagInfo->tagId, &allPropCnt, &allPropKeys)))
			return NULL;
		NEED_FREE_N(allPropKeys, allPropCnt);
		props = ht_create(allPropCnt);
		for(iProp = 0; iProp < allPropCnt; iProp++)
		{
			char *valueStr = NULL;
			char *key = allPropKeys[iProp];
			if(!is_property_blacklisted(key)
				&& tag_info_ask_prop_value(tagInfo, key, &valueStr))
			{
				NEED_FREE(valueStr);
				DTLLOG(TAG_INFO_FMT"Property[%d/%d]:'%s'='%s'",
						 TAG_INFO_ARGS(tagInfo), iProp + 1, allPropCnt, key, valueStr);
				if(!string_isNullOrSpace(valueStr))
				{
					ht_set_chars(props, key, valueStr);
				}
				TC_FREE(valueStr);
			}
			TC_FREE(key);
		}
		TC_FREE(allPropKeys);
		DBGLOG(TAG_INFO_FMT" has %d properties which contains value", TAG_INFO_ARGS(tagInfo), props->count);
		PROPS(tagInfo) = props;
	}
	return PROPS(tagInfo);
}

string_list_p pref_get_values(const char *key)
{
	int count;
	char **values = NULL;
	string_list_p blacklist = string_list_new();
	if(ITK_EVAL(PREF_ask_char_values(key, &count, &values)))
	{
		NEED_FREE(values);
		if(count > 0)
		{
			int i;
			for(i=0; i<count; i++)
			{
				DBGLOG("%s[%d/%d]='%s'", key, i+1, count, values[i]);
				string_list_add(blacklist, values[i]);
			}
		}
		else
		{
			DBGLOG("%s has 0 value", key);
		}
		TC_FREE(values);
	}
	return blacklist;
}
kvl_p pref_get_protect_attributes()
{
	int cntAttributes = -1;
	char **prefValues = NULL;
	kvl_p list = NULL;
	if(ITK_EVAL(PREF_ask_char_values(PREF_PROTECT_ATTRS, &cntAttributes, &prefValues))
		&& cntAttributes > 0)
	{
		int i;
		NEED_FREE(prefValues);
		DBGLOG("Found %d attributes from preferences", cntAttributes);
		list = kvl_new(cntAttributes);
		for(i=0; i<cntAttributes; i++)
		{
			char *attr = prefValues[i];
			char *defaultValue = attr;
			//
			while((*defaultValue)!='\0')
			{
				if((*defaultValue)==':')
				{
					//get default value from key string
					defaultValue[0] = '\0';
					defaultValue++;
					break;
				}
				defaultValue++;
			}
			DBGLOG("==>Key='%s' Default='%s'", attr, defaultValue);
			kvl_add(list, attr, defaultValue);
		}
		TC_FREE(prefValues);
	}
	else
	{
		DBGLOG("Failed to load %s from Teamcenter Preferences", PREF_PROTECT_ATTRS);
	}
	return list;
}
const static struct {
    component_t      compEnum;
    const char *compName;
} componentConversion [] = {
	{ dlp_exit_comp,		COMPONENT_DLP_SERVER_EXIT	},
    { wf_handler_comp,		COMPONENT_WF_HANDLER		},
    { set_tool_comp,		COMPONENT_CHECKIN_HANDLER	},
    { checkin_handler_comp, COMPONENT_SET_TOOLUSED		}
};
solution_info get_solution_info()
{
	static struct solution_info_s *solution = NULL;
	if(solution == NULL )
	{
		const char *solutionString = NULL;
		TC_ALLOC(solution, struct solution_info_s);
		//initialize
		solution->type = solution_non;
		solution->components = no_component;
		if((solutionString = pref_get_char(PREF_SOLUTION_TYPE, SOLUTION_AT_REST))!=NULL)
		{
			if(_stricmp(solutionString, SOLUTION_AT_REST)==0)
			{
				solution->type = solution_atRest;
				solution->components = (component_t)(
											  wf_handler_comp
											| set_tool_comp
											| checkin_handler_comp);
			}
			else if(_stricmp(solutionString, SOLUTION_IN_MOTION)==0)
			{
				solution->type = solution_inMotion;
				solution->components = (component_t)(
											  dlp_exit_comp
											| checkin_handler_comp);
			}
			else
			{
				DBGLOG("Invalid Solution Type!Please use '"SOLUTION_AT_REST"' OR '"SOLUTION_IN_MOTION"'(without quote)");
			}
		}//if(pref_get_char(PREF_SOLUTION_TYPE, NULL))

		/*TODO:TBD:in current version, doesn't allow customer to set components by themselves
		if(solution->type == solution_non)
		{
			int n = -1;
			char **fs = NULL;
			DBGLOG("Trying to load solution info by '"PREF_ENABLED_COMPONENTS"' ...");
			if(ITK_EVAL(PREF_ask_char_values(PREF_ENABLED_COMPONENTS, &n, &fs)))
			{
				int i;
				NEED_FREE(fs);
				DBGLOG("Found %d Components in Preference", n);
				if(n>0)
				{
					//by default, the solution is at rest unless features contains DLP solution
					for(i=0; i<n; i++)
					{
						int j=0;
						component_t comp = no_component;
						DBGLOG("==>%s", fs[i]);
						for(j=0; j<sizeof(componentConversion); j++)
						{
							if(stricmp(fs[i], componentConversion[j].compName)==0)
							{
								comp = componentConversion[j].compEnum;
								break;
							}
						}
						if(comp == no_component)
						{
							DBGLOG("Unknown Components-'%s'", fs[i]);
						}
						else
						{
							solution->components = (component_t)(solution->components|comp);
						}
					}
					//TODO: decide solution type by components
					solution->type = (solution->components & set_tool_comp)
						?solution_atRest
						:solution_inMotion;
				}
				TC_FREE(fs);
			}
			else
			{
				ERRLOG("Failed to load %s from Teamcenter Preferences", PREF_ENABLED_COMPONENTS);
			}
		}//if(solution->type == solution_non)
		*/

		//debug
		if(solution->type!=solution_non)
		{
			SYSLOG("Solution = %s", solution->type==solution_inMotion ? SOLUTION_IN_MOTION : SOLUTION_AT_REST);
			//TODO:enumerate components
			MOCK_FREE(solution);//cache
		}
		else
		{
			TC_FREE(solution);
		}
	}//if(solution == NULL )
	return solution;
}

char *imanfile_get_read_ticket(tag_info_ro fileInfo)
{
	char *ret = NULL;
	int *fails = NULL;
	char **tickets = NULL;
	if (ITK_EVAL(IMF_get_read_file_tickets(1, &fileInfo->tagId, "", &fails, &tickets)))
	{
		ret = TC_DUP(tickets[0]);
		DBGLOG(TAG_INFO_FMT ":ReadTicket='%s'", TAG_INFO_ARGS(fileInfo), ret);
	}
	else
	{
		DBGLOG(TAG_INFO_FMT "GetReadTicket failed with error-%d", TAG_INFO_ARGS(fileInfo), fails[0]);
	}
	return ret;
}

#define NXL_FILE_HEADER "NXLFMT"
#define NXL_FILE_HEADER_CNT sizeof(NXL_FILE_HEADER)
BOOL imanfile_is_protected(tag_info_ro fileInfo, BOOL *isProtected)
{
	BOOL ret = FALSE;
	IMF_file_t fileDescriptor;
	if(fileInfo != NULL
		&& ITK_EVAL(IMF_ask_file_descriptor(fileInfo->tagId, &fileDescriptor)))
	{
		if(ITK_EVAL(IMF_open_file(fileDescriptor, SS_RDONLY)))
		{
			int readCnt;
			char buffer[SS_MAXLLEN] = "";
			char *line = NULL;
			//for binary dataset
			if(ITK_EVAL(IMF_read_file(fileDescriptor, NXL_FILE_HEADER_CNT, SS_CHAR, &readCnt, buffer)))
			{
				DBGLOG(TAG_INFO_FMT":IMF_read_file(%p) returns %d chars('%s'), Expected='"NXL_FILE_HEADER"'", TAG_INFO_ARGS(fileInfo), fileDescriptor, readCnt, buffer);
			}
			//for text dataset
			else if(ITK_EVAL(IMF_read_file_line2(fileDescriptor, &line)))
			{
				NEED_FREE(line);
				DBGLOG(TAG_INFO_FMT":IMF_read_file_line(%p) returns first line(first %d chars='%.*s'), Expected='"NXL_FILE_HEADER"'", TAG_INFO_ARGS(fileInfo), fileDescriptor, NXL_FILE_HEADER_CNT, NXL_FILE_HEADER_CNT, line);
				if (line != NULL)
				{
					strcpy_s(buffer, SS_MAXLLEN, line);
				}
				TC_FREE(line);
			}
			else
			{
				goto CLOSE_FILE;
			}
			*isProtected = string_starts_with(buffer, NXL_FILE_HEADER)!=FALSE;
			ret = TRUE;
CLOSE_FILE:
			ITK_EVAL(IMF_close_file(fileDescriptor));
		}
		else
		{
			DBGLOG(TAG_INFO_FMT":Failed to Open file descriptor(%d)", TAG_INFO_ARGS(fileInfo), fileDescriptor);
		}
	}
	else
	{
		DBGLOG(TAG_INFO_FMT":Failed to obtain file descriptor", TAG_INFO_ARGS(fileInfo));
	}
	return ret;
}
BOOL imanfile_is_protected_fsc(tag_info_ro fileInfo, BOOL *isProtected) {
	BOOL ret = FALSE;
	char *readTicket = imanfile_get_read_ticket(fileInfo);
	if (!string_isNullOrSpace(readTicket))
	{
		char header[NXL_FILE_HEADER_CNT + 1] = "";
		int nread = 0;
		char *szError = NULL;
		if ((nread = FSC_ReadBytes_UTF(FSC_POLICY_TCDRM, readTicket, 0, NXL_FILE_HEADER_CNT, header, &szError)) < 0
			|| szError != NULL) {
			ERRLOG("calling FSC_ReadBytes_UTF() failed(%d): '%s'", nread, szError);
			if (PREF_WHOLE_FILE_DOWNLOAD_ENABLED())
			{
				ret = imanfile_is_protected(fileInfo, isProtected);
			}
		}
		else
		{
			DBGLOG("FSC_ReadBytes_UTF():nread=%d header='%s'", nread, header);
			*isProtected = string_starts_with(header, NXL_FILE_HEADER) != FALSE;
			ret = TRUE;
		}
		TC_FREE(szError);//TODO:how to free this string
	}
	TC_FREE(readTicket);
	return ret;
}

#define NXLV2_FILE_HEADER "NXLFMT@"
#define NXLV2_SECTION_SIZE 0x1000
#define NXLV2_TAGSECTION_START 0x3000

/*Find next char that is not in double quote*/
int FindNextChar(const char *str, int istart, char c)
{
	BOOL inquote = FALSE;
	int i = istart;
	if (str == NULL)
		return -1;

	while (str[i] != '\0') {
		if (!inquote&&str[i] == c) {
			//DBGLOG("Found '%c' at %d(from-%d)", c, i, istart);
			return i;
		}
		switch (str[i])
		{
		case '\\':
			i++;
			break;
		case '\"':
			inquote = !inquote;
			break;
		default:
			break;
		}
		i++;
	}
	DBGLOG("Failed to find '%c' from %d in '%s'", c, istart, str);
	return -1;
}
kvl_p load_json_tags(const char *json) {
	kvl_p tags = NULL;
	char *key = NULL;
	int len = strlen(json);
	int i = FindNextChar(json, 0, '{');
	if (i < 0) return tags;//invalid
	tags = kvl_new(5);
	
	//parse json
	while (i < len)
	{
		//search key value pair one by one
		int ikeystart, ikeyend;
		int iArrayStart, iArrayEnd;
		int ivalueStart, ivalueEnd;
		ikeystart = FindNextChar(json, i + 1, '\"');
		if (ikeystart < 0) break;//invalid
		ikeyend = FindNextChar(json, ikeystart + 1, '\"');
		if (ikeyend < 0) break;//invalid
		TC_FREE(key);
		if (ikeyend - ikeystart >= 2)
			key = TC_NCOPY(&(json[ikeystart + 1]), ikeyend - ikeystart - 1);
		else
			key = TC_DUP("");

		//found key, start search value array
		iArrayStart = FindNextChar(json, ikeyend + 1, '[');
		if (iArrayStart < 0) break;//invalid
		iArrayEnd = FindNextChar(json, iArrayStart, ']');
		if (iArrayEnd < 0) break;//invalid
		ivalueEnd = iArrayStart;
		while ((ivalueStart = FindNextChar(json, ivalueEnd + 1, '\"')) < iArrayEnd)
		{
			if (ivalueStart < 0) break;
			ivalueEnd = FindNextChar(json, ivalueStart + 1, '\"');
			if (ivalueEnd < 0) break;//invalid;
			if (ivalueEnd - ivalueStart >= 2)
			{
				char *value = TC_NCOPY(&(json[ivalueStart + 1]), ivalueEnd - ivalueStart - 1);
				//add key value
				kvl_add(tags, key, value);
				DBGLOG("'%s'='%s'", key, value);
				TC_FREE(value);
			}
		}

		//end of array
		i = iArrayEnd + 1;
	}
	TC_FREE(key);

	return tags;

}
BOOL imanfile_read_metadata(tag_info_ro fileInfo, kvl_p *tags) {
	BOOL ret = FALSE;
	IMF_file_t fileDescriptor;
	if (fileInfo != NULL
		&& ITK_EVAL(IMF_ask_file_descriptor(fileInfo->tagId, &fileDescriptor)))
	{
		if (ITK_EVAL(IMF_open_file(fileDescriptor, SS_RDONLY)))
		{
			int pos = 0;
			int readCnt;
			char buffer[NXLV2_SECTION_SIZE+1] = "";
			//for binary dataset
			if (!ITK_EVAL(IMF_read_file(fileDescriptor, NXLV2_SECTION_SIZE, SS_CHAR, &readCnt, buffer)))
			{
				//failed to read first section
				goto CLOSE_FILE;
			}
			DBGLOG(TAG_INFO_FMT":IMF_read_file(%p) returns %d chars(first %d chars='%.*s'), Expected(%d)='"NXLV2_FILE_HEADER"'"
				, TAG_INFO_ARGS(fileInfo), fileDescriptor, readCnt, sizeof(NXLV2_FILE_HEADER), sizeof(NXLV2_FILE_HEADER), buffer, NXLV2_SECTION_SIZE);
			pos += NXLV2_SECTION_SIZE;
			if (!string_starts_with(buffer, NXLV2_FILE_HEADER)) {
				//only process protected file
				DBGLOG("skip reading for non-protected file");
				goto CLOSE_FILE;
			}
			//read tag section
			while (pos <= NXLV2_TAGSECTION_START)
			{
				DBGLOG("Reading section %#X...", pos);
				if (!ITK_EVAL(IMF_read_file(fileDescriptor, NXLV2_SECTION_SIZE, SS_CHAR, &readCnt, buffer)))
				{
					//failed to read section
					goto CLOSE_FILE;
				}
				pos += NXLV2_SECTION_SIZE;
			}
			DBGLOG("first %d chars='%.*s'", 100, 100, buffer);
			*tags = load_json_tags(buffer);

			ret = (*tags) != NULL;
		CLOSE_FILE:
			ITK_EVAL(IMF_close_file(fileDescriptor));
		}
		else
		{
			DBGLOG(TAG_INFO_FMT":Failed to Open file descriptor(%d)", TAG_INFO_ARGS(fileInfo), fileDescriptor);
		}
	}
	else
	{
		DBGLOG(TAG_INFO_FMT":Failed to obtain file descriptor", TAG_INFO_ARGS(fileInfo));
	}
	return ret;
}
BOOL imanfile_read_metadata_fsc(tag_info_ro fileInfo, kvl_p *tags) {
	BOOL ret = FALSE;
	if (fileInfo != NULL)
	{
		int readCnt;
		char buffer[NXLV2_SECTION_SIZE + 1] = "";
		char *readTicket = imanfile_get_read_ticket(fileInfo);
		if (!string_isNullOrSpace(readTicket))
		{
			char *szErrorUtf = NULL;
			if ((readCnt = FSC_ReadBytes_UTF(FSC_POLICY_TCDRM, readTicket, 0, sizeof(NXLV2_FILE_HEADER), buffer, &szErrorUtf)) > 0)
			{
				DBGLOG(TAG_INFO_FMT":FSC_ReadBytes_UTF() returns %d chars(first %d chars='%.*s'), Expected(%d)='"NXLV2_FILE_HEADER"'"
					, TAG_INFO_ARGS(fileInfo), readCnt, sizeof(NXLV2_FILE_HEADER), sizeof(NXLV2_FILE_HEADER), buffer, sizeof(NXLV2_FILE_HEADER));
				if (!string_starts_with(buffer, NXLV2_FILE_HEADER)) {
					//only process protected file
					DBGLOG("skip reading for non-protected file");
					goto CLEANUP;
				}
				DBGLOG("Reading section %#X...", NXLV2_TAGSECTION_START);
				if ((readCnt = FSC_ReadBytes_UTF(FSC_POLICY_TCDRM, readTicket, NXLV2_TAGSECTION_START, NXLV2_SECTION_SIZE, buffer, &szErrorUtf)) > 0)
				{
					DBGLOG(TAG_INFO_FMT":FSC_ReadBytes_UTF() returns %d chars(first %d chars='%.*s')"
						, TAG_INFO_ARGS(fileInfo), readCnt, 100, 100, buffer);
					*tags = load_json_tags(buffer);

					ret = (*tags) != NULL;
					goto CLEANUP;
				}
			}
			ERRLOG("calling FSC_ReadBytes_UTF() failed(%d): '%s'", readCnt, szErrorUtf);
		CLEANUP:
			TC_FREE(szErrorUtf);
		}
		TC_FREE(readTicket);
		if (!ret && PREF_WHOLE_FILE_DOWNLOAD_ENABLED())
		{
			ret = imanfile_read_metadata(fileInfo, tags);
		}
	}
	return ret;
}

logical imanfile_rename(tag_info_ro fileInfo, char *newName)
{
	if(fileInfo != NULLTAG)
	{
		if(ITK_EVAL(AOM_refresh(fileInfo->tagId, TRUE)))
		{
			//validate new name;
			char *inputName = TC_DUP(newName);
			int nameLen = strlen(newName);
			if(nameLen > IMF_filename_size_c)
			{
				//the new name exceed the length limit, Teamcenter will truncate it automatically
				//in order to make sure the original .xxx.nxl extension is not lost, we need truncate it by ourselves
				int i;
				int iext = -1, inxl = -1;
				for(i = nameLen-1; i >= 0; i--)
				{
					if(newName[i] != '.') continue;
					//found a dot
					if(inxl > 0)
					{
						//found original extension
						iext = i;
						break;
					}
					else if(_stricmp(newName + i, NXL_SUFFIX)==0)
					{
						//check if .nxl extension, as we only revise the new name when it's a .nxl file
						inxl = i;
					}
					else
					{
						break;
					}
				}
				if(inxl > 0 && iext > 0)
				{
					int itruncate = IMF_filename_size_c - (nameLen - iext);
					while(TRUE)
					{
						newName[itruncate] = newName[iext];
						if(newName[iext] == '\0') break;
						itruncate++;
						iext++;
					}
					DBGLOG(TAG_INFO_FMT":Truncated inputName('%s') into '%s'", TAG_INFO_ARGS(fileInfo), inputName, newName);
				}
				else
				{
					DBGLOG(TAG_INFO_FMT":Failed to truncate inputName('%s')", TAG_INFO_ARGS(fileInfo), newName);
				}
			}
			if(ITK_EVAL(IMF_set_original_file_name2(fileInfo->tagId, newName)))
			{
				if(ITK_EVAL(AOM_save_without_extensions(fileInfo->tagId)))
				{
					if(ITK_EVAL(AOM_unlock(fileInfo->tagId)))
					{
						DBGLOG(TAG_INFO_FMT":Renamed to '%s'", TAG_INFO_ARGS(fileInfo), newName);
					}
				}
			}
			TC_FREE(inputName);
		}
	}
	return true;
}

tag_list_p TAG_LIST_new()
{
	tag_list_p list;
	if(TC_ALLOC(list, struct tag_list_s) != NULL)
	{
		list->tags = NULL;
		list->count = 0;
	}
	return list;
}
void TAG_LIST_free(tag_list_p list)
{
	if(list != NULL)
	{
		TC_FREE(list->tags);
		TC_FREE(list);
	}
}

wf_args_p WF_ARGS_new()
{
	wf_args_p wfargs;
	TC_ALLOC(wfargs, struct _action_handler_args);
	wfargs->datasetTypes = string_list_new();
	wfargs->scheduledTime = 0;
	wfargs->relationTypes = TAG_LIST_new();
	wfargs->relationNames = string_list_new();
	wfargs->process_assembly = 0;
	wfargs->skip_nxl_datasets = 0;
	wfargs->operation = wfop_undefined;
	wfargs->workflowName = NULL;
	wfargs->datasets = TAG_LIST_new();
	wfargs->translations = TAG_LIST_new();
	wfargs->itemRevisions = TAG_LIST_new();
	wfargs->attachments = TAG_LIST_new();
	return wfargs;
}
logical TAG_LIST_contains(tag_list_p list, tag_t tag)
{
	logical ret = false;
	if(list!=NULL && tag != NULLTAG)
	{
		int i;
		for(i=0; i<list->count; i++)
		{
			if(list->tags[i] == tag)
			{
				return true;
			}
		}
	}
	return ret;
}
void TAG_LIST_add(tag_list_p list, tag_t tag)
{
	if(list != NULL)
	{
		CALL_DTL(TC_REALLOC(list->tags, tag_t, list->count+1));
		CALL_DTL(list->tags[list->count] = tag);
		CALL_DTL(list->count++);
	}
}
void WF_ARGS_free(wf_args_p wfargs)
{
	if(wfargs!=NULL)
	{
		string_list_free(wfargs->datasetTypes);
		string_list_free(wfargs->relationNames);
		TAG_LIST_free(wfargs->datasets);
		TAG_LIST_free(wfargs->translations);
		TAG_LIST_free(wfargs->itemRevisions);
		TAG_LIST_free(wfargs->relationTypes);
		TAG_LIST_free(wfargs->attachments);
		TC_FREE(wfargs);
	}
}

#define NXL_DATASET_PROP_IS_PROTECTED "nxl3_is_nxl_protected"
#define NXL_DATASET_PROP_VALUE "true"
logical dataset_is_nxl(tag_info_ro tagInfo)
{
	logical ret = false;
	if(tagInfo!=NULL)
	{
#ifdef DEBUG
		if(string_ends_with(tagInfo->name, ".nxl", 0))
		{
			ret = true;
		}
		else
#endif
		{
			ret = is_nxl_dataset_tag(tagInfo->tagId);
			DBGLOG(TAG_INFO_FMT"IsProtected=%d", TAG_INFO_ARGS(tagInfo), ret);
		}
	}
	return ret;
}
logical tag_info_ask_prop_value(tag_info_ro obj, const char *propName, char **oPropValue)
{
	int nvalues = 0;
	char **values = NULL;
	if (propName != NULL
		&& ITK_EVAL(AOM_ask_displayable_values(obj->tagId, propName, &nvalues, &values)))
	{
		if (nvalues > 0)
		{
			NEED_FREE(values);
			*oPropValue = TC_DUP(strings_join(values, nvalues, ";"));
			DBGLOG(TAG_INFO_FMT":'%s'='%s'(%d values)", TAG_INFO_ARGS(obj), propName, *oPropValue, nvalues);
			TC_FREE(values);
			return true;
		}
	}
	DBGLOG(TAG_INFO_FMT":'%s' has no value", TAG_INFO_ARGS(obj), propName);
	return false;

}
logical is_nxl_dataset_tag(tag_t tag)
{
	logical ret = false;
	int nValues = 0;
	char **values = NULL;
			
	if(ITK_EVAL(AOM_ask_displayable_values(tag, NXL_DATASET_PROP_IS_PROTECTED, &nValues, &values)))
	{
		NEED_FREE(values);
		if(nValues > 0)
		{
			int iValue;
			for(iValue = 0; iValue < nValues; iValue++)
			{
				DTLLOG("==>Value[%d]='%s'", iValue, values[iValue]);
				if(0 == _stricmp(values[iValue], NXL_DATASET_PROP_VALUE))
				{
					ret = true;
					break;
				}
			}
		}
		else
		{
			DBGLOG("[#%u]"NXL_DATASET_PROP_IS_PROTECTED" has no value", tag);
		}

		TC_FREE(values);
	}
	return ret;
}
char *dataset_get_type(tag_info_ro dsInfo) {
	char *dsType = NULL;
	if (!ITK_EVAL(WSOM_ask_object_type2(dsInfo->tagId, &dsType)))
		return NULL;
	NEED_FREE(dsType);
	return dsType;
}
int dataset_delete_refs(tag_info_ro dsInfo, tag_list_p tags, string_list_ro refNames) {
	int ndeletedRefs = 0;
	int ndeletedFiles = 0;
	if (tags != NULL		&& refNames != NULL		)
	{
		if (tags->count > 0 && refNames->count > 0 && tags->count == refNames->count)
		{
			int iref;
			BOOL locked = FALSE;
			logical modifiable = false;
			ITK_EVAL(POM_modifiable(dsInfo->tagId, &modifiable));
			DBGLOG(TAG_INFO_FMT":modifiable=%d", TAG_INFO_ARGS(dsInfo), modifiable);
			if (!modifiable)
			{
				if (ITK_EVAL(AOM_refresh(dsInfo->tagId, TRUE)))
				{
					locked = TRUE;
				}
			}
			//ITK_CALL(AE_purge_dataset_revs(dsInfo->tagId));
			for (iref = 0; iref < tags->count; iref++)
			{
				DBGLOG(TAG_INFO_FMT ":Removing Named Reference[%d/%d]:refName=%s tag=%u", TAG_INFO_ARGS(dsInfo), iref + 1, tags->count, refNames->items[iref], tags->tags[iref]);
				if (ITK_EVAL(AE_remove_dataset_named_ref_by_tag2(dsInfo->tagId, refNames->items[iref], tags->tags[iref])))
				{
					ndeletedRefs++;
				}
			}
			if (ndeletedRefs)
			{
				//ITK_EVAL(POM_save_instances(1, &(dsInfo->tagId), FALSE));
				suppress_message_handler(AE_save_myself_dataset_msg, TRUE);
				ITK_CALL(AE_save_myself(dsInfo->tagId));
				suppress_message_handler(AE_save_myself_dataset_msg, FALSE);
				for (iref = 0; iref < tags->count; iref++)
				{
					if (imf_delete_file(tags->tags[iref]))
					{
						ndeletedFiles++;
					}
				}
			}
			if (locked)
			{
				ITK_EVAL(AOM_refresh(dsInfo->tagId, FALSE));
			}
		}
	}
	DBGLOG(TAG_INFO_FMT ":%d references and %d files are deleted", TAG_INFO_ARGS(dsInfo), ndeletedRefs, ndeletedFiles);
	return ndeletedRefs;
}
BOOL tag_info_describe_attribute(tag_info_ro tagInfo, const char *attrName, tag_t *attrId, int *attrType, BOOL *isArray) {
	BOOL ret = FALSE;
	if (ITK_EVAL(POM_attr_id_of_attr(attrName, tagInfo->className, attrId))
		&& *attrId != NULLTAG)
	{
		char **names = NULL;
		int *types, *max_string_lengths, *lengths, *descriptors, *attrFailures;
		tag_t *referenced_classes;
		if (ITK_EVAL(POM_describe_attrs(tagInfo->classId, 1, attrId, &names, &types, &max_string_lengths, &referenced_classes, &lengths, &descriptors, &attrFailures)))
		{
			char *refClassName = NULL;
			NEED_FREE(names);
			NEED_FREE(types);
			NEED_FREE(max_string_lengths);
			NEED_FREE(referenced_classes);
			NEED_FREE(lengths);
			NEED_FREE(descriptors);
			NEED_FREE(attrFailures);
			if (referenced_classes[0] != NULLTAG)
			{
				if(ITK_EVAL(POM_name_of_class(referenced_classes[0], &refClassName)))
					NEED_FREE(refClassName);
			}
			DBGLOG(TAG_INFO_FMT":AttrName='%s' Id=%d type=%d maxStringLen=%d refClass=%u(%s) len=%d descriptor=%#X attrFailure=%d", TAG_INFO_ARGS(tagInfo)
				, names[0], attrId, types[0], max_string_lengths[0], referenced_classes[0], refClassName, lengths[0], descriptors[0], attrFailures[0]);

			*attrType = types[0];
			*isArray = lengths[0] > 1;
			ret = TRUE;

			TC_FREE(refClassName);
			TC_FREE(names);
			TC_FREE(types);
			TC_FREE(max_string_lengths);
			TC_FREE(referenced_classes);
			TC_FREE(lengths);
			TC_FREE(descriptors);
			TC_FREE(attrFailures);
		}
	}
	return ret;
}
BOOL dataset_set_attr_value_string(tag_info_ro dsInfo, const char *attrName, tag_t propId, const char *newValue) {
	BOOL ret = FALSE;
	logical modifiable = FALSE;
	BOOL locked = FALSE;
	ITK_EVAL(POM_modifiable(dsInfo->tagId, &modifiable));
	DBGLOG(TAG_INFO_FMT":modifiable=%d", TAG_INFO_ARGS(dsInfo), modifiable);
	if (!modifiable)
	{
		if (ITK_EVAL(AOM_refresh(dsInfo->tagId, TRUE)))
		{
			locked = TRUE;
		}
	}
	if (ITK_EVAL(POM_set_attr_string(1, &(dsInfo->tagId), propId, newValue))
		//use POM_save_instances to save object changes instead of AE_save_myself to avoid trigger the nxl3_dataset_save_post handler
		//don't use AOM_save() which will create a revision on dataset, the following changes will be dropped after cancel checkout
		&& ITK_EVAL(POM_save_instances(1, &(dsInfo->tagId), FALSE)))
	{
		DBGLOG(TAG_INFO_FMT":attr:'%s' has been updated as '%s'", TAG_INFO_ARGS(dsInfo), attrName, newValue);
		ret = TRUE;
	}
	else
	{
		ERRLOG(TAG_INFO_FMT":FAILED to update atrr-'%s' as '%s' %d", TAG_INFO_ARGS(dsInfo), attrName, newValue);
	}
	if (locked)
	{
		ITK_EVAL(AOM_refresh(dsInfo->tagId, FALSE));
	}
	return ret;
}
BOOL dataset_update_attr_value(tag_info_ro dsInfo, const char *attrName, const char *newValue, BOOL overrideOnExist) {
	BOOL ret = FALSE;
	char *oldValue = NULL;
	if (tag_info_ask_prop_value(dsInfo, attrName, &oldValue))
	{
		NEED_FREE(oldValue);
		DTLLOG(TAG_INFO_FMT":'%s'='%s'",
			TAG_INFO_ARGS(dsInfo), attrName, oldValue);
		if (string_isNullOrSpace(oldValue)
			|| (strcmp(oldValue, newValue) != 0 && overrideOnExist))
		{
			//update prop value when old value is empty or not equal
			tag_t propId = NULLTAG;
			BOOL isArray = FALSE;
			int propType;
			if (tag_info_describe_attribute(dsInfo, attrName, &propId, &propType, &isArray))
			{
				switch (propType)
				{
				case POM_string:
					if (!isArray)
					{
						ret = dataset_set_attr_value_string(dsInfo, attrName, propId, newValue);
						break;
					}
				default:
					DBGLOG("unsupported attribute type-%d(array=%d)", propType, isArray);
					break;
				}
			}
			else
			{
				DBGLOG(TAG_INFO_FMT":cannot find attribute-'%s'", TAG_INFO_ARGS(dsInfo), attrName);
			}
		}
		TC_FREE(oldValue);
	}
	return ret;
}
BOOL dataset_set_is_protected(tag_info_ro dsInfo, logical newValue)
{
	BOOL ret = FALSE;
	//set value to false will be done only in remove protection translation request
	tag_t propId = NULLTAG;
	BOOL locked = FALSE;
	logical modifiable = false;
	logical oldValue;
	ITK_EVAL(POM_attr_id_of_attr(NXL_DATASET_PROP_IS_PROTECTED, dsInfo->className, &propId));
	if(propId == NULLTAG)
	{
		return FALSE;
	}
	else
	{
		logical isNull, isEmpty;
		if(ITK_EVAL(POM_ask_attr_logical(dsInfo->tagId, propId, &oldValue, &isNull, &isEmpty)))
		{
			DBGLOG(TAG_INFO_FMT":"NXL_DATASET_PROP_IS_PROTECTED"=%d", TAG_INFO_ARGS(dsInfo), oldValue);
			if(oldValue == newValue)
				return TRUE;
		}
	}
	ITK_EVAL(POM_modifiable(dsInfo->tagId, &modifiable));
	DBGLOG(TAG_INFO_FMT":modifiable=%d", TAG_INFO_ARGS(dsInfo), modifiable);
	if(!modifiable)
	{
		if(ITK_EVAL(AOM_refresh(dsInfo->tagId, TRUE)))
		{
			locked = TRUE;
		}
	}
	if(ITK_EVAL(POM_set_attr_logical(1, &(dsInfo->tagId), propId, newValue))
		//use POM_save_instances to save object changes instead of AE_save_myself to avoid trigger the nxl3_dataset_save_post handler
		//don't use AOM_save() which will create a revision on dataset, the following changes will be dropped after cancel checkout
		&& ITK_EVAL(POM_save_instances(1, &(dsInfo->tagId), FALSE)))
	{
		DBGLOG(TAG_INFO_FMT":Updated "NXL_DATASET_PROP_IS_PROTECTED" from %d to %d", TAG_INFO_ARGS(dsInfo), oldValue, newValue);
		ret = TRUE;
	}
	else
	{
		ERRLOG(TAG_INFO_FMT":FAILED to update "NXL_DATASET_PROP_IS_PROTECTED" from %d to %d", TAG_INFO_ARGS(dsInfo), oldValue, newValue);
	}
	if(locked)
	{
		ITK_EVAL(AOM_refresh(dsInfo->tagId, FALSE));
	}
	return ret;
}
const char *get_tc_bin()
{
	static char tcbin[MAX_PATH] = "";
	if(string_isNullOrSpace(tcbin))
	{
#if defined(WNT)
		if(!get_module_dir(TARGETFILENAME, tcbin, MAX_PATH))
#elif defined(__linux__)
		if(!get_lib_dir((void*)get_tc_bin, tcbin, MAX_PATH))
#endif
		{
			tcbin[0] = '\0';
		}
	}
	DBGLOG("Teamcenter Bin Folder=%s", tcbin);
	return tcbin;
}

BOOL pref_get_logical(const char *key, BOOL defaultValue)
{
	logical value;
	int nvalues = 0;
	if(ITK_EVAL(PREF_ask_value_count(key, &nvalues))
		&& nvalues > 0
		&& ITK_EVAL(PREF_ask_logical_value(key, 0, &value)))
	{
		DBGLOG("%s=%d", key, value);
		return value;
	}
	else
	{
		DBGLOG("%s=%d(default)", key, defaultValue);
		return defaultValue;
	}
}
const char *pref_get_char(const char *key, const char *defaultValue)
{
	static char *value = NULL;
	TC_FREE(value);
	if(ITK_EVAL(PREF_ask_char_value(key, 0, &value))
		&& value != NULL)
	{
		NEED_FREE(value);
		DBGLOG("%s=%s", key, value);
	}
	else
	{
		value = TC_DUP(defaultValue);
		DBGLOG("%s='%s'(default)", key, value);
	}
	return value;
}
const char *pref_get_char_cached(const char *key, const char *defaultValue)
{
	static hashtable_p cache = NULL;
	const char *cachedValue = NULL;
	const char *tmpValue;
	if(cache == NULL)
	{
		cache = ht_create(10);
	}
	else
	{
		if(ht_tryget_chars(cache, key, &cachedValue))
		{
			//found cached value
			DBGLOG("[CACHED]%s=%s", key, cachedValue);
			return cachedValue;
		}
	}
	//search new value
	if((tmpValue = pref_get_char(key, defaultValue)) != NULL)
	{
		//update cache
		cachedValue = ht_set_chars(cache, key, tmpValue);
	}
	return cachedValue;
}

kvl_p dataset_get_protect_attributes(tag_info_ro dsInfo)
{
	kvl_p keys = pref_get_protect_attributes();
	if(keys != NULL
		&& keys->count > 0)
	{
		int ikey;
		for(ikey=0; ikey<keys->count; ikey++)
		{
			char *propValue = NULL;
			const char *key = keys->keys[ikey];
			if(key != NULL
				&& tag_info_ask_prop_value(dsInfo, key, &propValue))
			{
				NEED_FREE(propValue);
				DTLLOG(TAG_INFO_FMT"-"PREF_PROTECT_ATTRS"[%d/%d]:'%s'='%s'",
							TAG_INFO_ARGS(dsInfo), ikey + 1, keys->count, key, propValue);
				if(!string_isNullOrSpace(propValue))
				{
					kvl_setOrAdd(keys, key, propValue);
				}
				TC_FREE(propValue);
			}
		}
	}
	return keys;
}
BOOL ask_reference_name(tag_t fileTag, tag_t dsTag, char **referenceName)
{
	BOOL ret = FALSE;
	*referenceName = NULL;
	if(fileTag != NULLTAG
		&& dsTag != NULLTAG)
	{
		int nrefs;
		if(ITK_EVAL(AE_ask_dataset_ref_count(dsTag, &nrefs)))
		{
			int iref;
			DBGLOG("There are %d named references for this Dataset", nrefs);
			for(iref=0; iref < nrefs; iref++)
			{
				char *refName;
				AE_reference_type_t refType;
				tag_t refTag;
				if(ITK_EVAL(AE_find_dataset_named_ref2(dsTag, iref, &refName, &refType, &refTag)))
				{
					NEED_FREE(refName);
					DBGLOG("File[%d/%d]RefName='%s' RefType=%d Tag=%d ExpectedFileTag=%d", iref+1, nrefs, refName, refType, refTag, fileTag);
					if(refTag == fileTag)
					{
						*referenceName = TC_DUP(refName);
						ret = TRUE;
						TC_FREE(refName);
						break;//TODO
					}
					TC_FREE(refName);
				}
			}
		}
	}
	return ret;
}
int find_relation_types(char * const *inputNames, int ninputs, tag_list_p relationTypes, string_list_p relationNames)
{
	int nTypes = 0;
	if(inputNames != NULL
		&& ninputs > 0
		&& relationTypes != NULL
		&& relationNames != NULL)
	{
		int iName;
		for(iName=0; iName < ninputs; iName++)
		{
			tag_t relationType = NULLTAG;
			if(ITK_EVAL(GRM_find_relation_type(inputNames[iName], &relationType)))
			{
				TAG_LIST_add(relationTypes, relationType);
				//cache relation name
				string_list_add(relationNames, inputNames[iName]);
				nTypes++;
				DBGLOG("[%d/%d]('%s')='%u'", iName+1, ninputs, inputNames[iName], relationType);
			}
		}
	}
	return nTypes;
}
/*
	Snapshot Module
*/
#define PROP_SNAPSHOT_FILE "nxl3_snapshot_file"
static tag_t snapshot_prop_id;
#define SNAPSHOT_IS_ENABLED(tagInfo) (tagInfo!=NULL && ITK_EVAL(POM_attr_id_of_attr(PROP_SNAPSHOT_FILE, tagInfo->className, &snapshot_prop_id)) && snapshot_prop_id!=NULLTAG)

static hashtable_p snapshot_load(tag_info_ro tagInfo)
{
	//create prop file
	tag_t fileTag = NULLTAG;
	if(ITK_EVAL(AOM_ask_value_tag(tagInfo->tagId, PROP_SNAPSHOT_FILE, &fileTag)))
	{
		if(fileTag != NULLTAG)
		{
			IMF_file_t fileDescriptor;
			if(ITK_EVAL(IMF_ask_file_descriptor(fileTag, &fileDescriptor)))
			{
				if(ITK_EVAL(IMF_open_file(fileDescriptor, SS_RDONLY)))
				{
					int nline = 0;
					char *line = NULL;
					hashtable_p props = ht_create(10);
					DBGLOG("Reading File-%u", fileTag);
					while(ITK_EVAL(IMF_read_file_line2(fileDescriptor, &line))
						&& line != NULL)
					{
						int ichar = strlen(line);
						nline++;
						while((line[ichar-1] == '\n'))
						{
							line[ichar-1] = '\0';
							ichar--;
						}
						DBGLOG("[%d]'%s'", nline, line);
						//split line to key-value pair
						for(ichar=0; line[ichar]!='\0'; ichar++)
						{
							if(line[ichar]==':')
							{
								//found seperator
								const char *key = line;
								const char *val = &(line[ichar+1]);
								line[ichar] = '\0';
								DBGLOG("==>'%s'='%s'", key, val);
								if(!string_isNullOrSpace(val))
								{
									ht_set_chars(props, key, val);
								}
								break;
							}
						}
						TC_FREE(line);
					}
					ITK_EVAL(IMF_close_file(fileDescriptor));
					DBGLOG("Loaded %d Properties from %d lines", props->count, nline);
					return props;
				}
			}
		}
		else
		{
			DBGLOG("Property-'"PROP_SNAPSHOT_FILE"' is not set");
		}
	}
	return NULL;
}

static BOOL snapshot_set_prop(tag_t objTag, const char *className, tag_t newFileTag)
{
	BOOL updated = FALSE;
	//create prop file
	tag_t oldFileTag = NULLTAG;
	//obtain old value
	ITK_EVAL(AOM_ask_value_tag(objTag, PROP_SNAPSHOT_FILE, &oldFileTag));
	if(oldFileTag != newFileTag)
	{
		tag_t toBeDeletedFile = newFileTag;//if update failed, the new file tag should be deleted
		//set new value
		if(ITK_EVAL(POM_refresh_instances(1, &objTag, NULLTAG, POM_modify_lock)))
		{
			if(ITK_EVAL(POM_set_attr_tag(1, &objTag, snapshot_prop_id, newFileTag))
				&& ITK_EVAL(POM_save_instances(1, &objTag, FALSE)))
			{
				DBGLOG("Updated-"PROP_SNAPSHOT_FILE":%u=>%u", oldFileTag, newFileTag);
				updated = TRUE;
				toBeDeletedFile = oldFileTag;
			}
			//UNLOCK
			ITK_EVAL(POM_refresh_instances(1, &objTag, NULLTAG, POM_no_lock));
		}
		//clean old value
		if(toBeDeletedFile != NULLTAG)
		{
			char *path = NULL;
			DBGLOG("Deleting: %u", toBeDeletedFile);
			ITK_EVAL(AOM_lock_for_delete(toBeDeletedFile));
			ITK_EVAL(IMF_ask_file_pathname2(toBeDeletedFile, SS_MACHINE_TYPE, &path));
			if(ITK_EVAL(AOM_delete(toBeDeletedFile)))
			{
				DBGLOG("Deleted: %s", path);
			}
			else
			{
				DBGLOG("DeletionFailed: %s", path);
				ITK_EVAL(AOM_unlock(toBeDeletedFile));
			}
			TC_FREE(path);
		}
	}
	else
	{
		DBGLOG("DuplicatedValue:"PROP_SNAPSHOT_FILE"='%u'", newFileTag);
	}
	return updated;
}

hashtable_p snapshot_load_props(tag_info_ro tagInfo)
{
	int i;
	int nprops;
	char **otherProps = NULL;
	hashtable_p ht = ht_create(10);
	kvl_p transferAttrs = pref_get_protect_attributes();
	//adding property value of transfer attributes
	if(transferAttrs != NULL && transferAttrs->count > 0)
	{
		for(i = 0; i < transferAttrs->count; i++)
		{
			char *valueStr = NULL;
			const char *key = transferAttrs->keys[i];
			if(key != NULL
				&& tag_info_ask_prop_value(tagInfo, key, &valueStr))
			{
				NEED_FREE(valueStr);
				DTLLOG(TAG_INFO_FMT"-"PREF_PROTECT_ATTRS"[%d/%d]:'%s'='%s'",
							TAG_INFO_ARGS(tagInfo), i + 1, transferAttrs->count, key, valueStr);
				if(!string_isNullOrSpace(valueStr))
				{
					ht_set_chars(ht, key, valueStr);
				}
				TC_FREE(valueStr);
			}
		}
	}
	//adding property value of additional attribute
	if(ITK_EVAL(PREF_ask_char_values(PREF_SNAPSHOT_PROPERTIES, &nprops, &otherProps)))
	{
		NEED_FREE(otherProps);
		if(nprops > 0)
		{
			for(i=0; i<nprops; i++)
			{
				char *key = otherProps[i];
				if(key != NULL)
				{
					if(transferAttrs != NULL
						&& strings_index_of(transferAttrs->keys, transferAttrs->count, key) >= 0)
					{
						DBGLOG(TAG_INFO_FMT"-"PREF_SNAPSHOT_PROPERTIES"[%d/%d]:'%s'-DUPLICATED",
									TAG_INFO_ARGS(tagInfo), i + 1, nprops, key);
					}
					else
					{
						char *valueStr = NULL;
						if(tag_info_ask_prop_value(tagInfo, key, &valueStr))
						{
							NEED_FREE(valueStr);
							DBGLOG(TAG_INFO_FMT"-"PREF_SNAPSHOT_PROPERTIES"[%d/%d]:'%s'='%s'",
										TAG_INFO_ARGS(tagInfo), i + 1, nprops, key, valueStr);
							if(!string_isNullOrSpace(valueStr))
							{
								ht_set_chars(ht, key, valueStr);
							}
							TC_FREE(valueStr);
						}
						else
						{
							DBGLOG(TAG_INFO_FMT"-"PREF_SNAPSHOT_PROPERTIES"[%d/%d]:'%s'-INVALID",
										TAG_INFO_ARGS(tagInfo), i + 1, nprops, key);
						}
					}
				}
				else
				{
					DBGLOG(TAG_INFO_FMT"-"PREF_SNAPSHOT_PROPERTIES"[%d/%d]:'%s'=NULL",
								TAG_INFO_ARGS(tagInfo), i + 1, nprops, key);
				}
			}
		}
		else
		{
			DBGLOG(PREF_SNAPSHOT_PROPERTIES" has 0 value");
		}
		TC_FREE(otherProps);
	}
	kvl_free(transferAttrs);
	return ht;
}

BOOL snapshot_capture(tag_info_ro tagInfo)
{
	if(SNAPSHOT_IS_ENABLED(tagInfo))
	{
		char tmpFile[MAX_PATH + 1] = "";
		DBGLOG(TAG_INFO_FMT":Taking Snapshot", TAG_INFO_ARGS(tagInfo));
		if(GetTempPath(MAX_PATH, tmpFile) > 0)
		{
			FILE *file = NULL;
			strcat_s(tmpFile, MAX_PATH, "Dataset.Checkout.bak");//TODO:make file name unique
			//file = fopen(tmpFile, "w");
			;
			if(fopen_s(&file, tmpFile, "w") == 0
				&& file != NULL)
			{
				char buff[SS_MAXLLEN + 1];
				int allRefCnt;
				tag_t *allRefs = NULL;
				int iprop;
				//for upload
				tag_t newFileTag = NULLTAG;
				IMF_file_t fileDescriptor;
				hashtable_p props = snapshot_load_props(tagInfo);
				DBGLOG("Generating:'%s'", tmpFile);
				//export dataset properties
				for(iprop=0; iprop<props->count; iprop++)
				{
					//DBGLOG("==>Exporing '%s'", props->kvps[iprop].key);
					_snprintf_s(buff, sizeof(buff), SS_MAXLLEN, "%s:%s\n", props->kvps[iprop].key, (const char *)(props->kvps[iprop].value));
					fputs(buff, file);
				}
				ht_free(props);
				//export named references properties
				if(tagInfo->rootType == type_dataset
					&& ((get_solution_info())->type==solution_inMotion)
					&& ITK_EVAL(AE_ask_dataset_named_refs(tagInfo->tagId, &allRefCnt, &allRefs)))
				{
					int iref = 0;
					for(iref=0; iref<allRefCnt; iref++)
					{
						char *fileName = NULL;
						if(ITK_EVAL(IMF_ask_original_file_name2(allRefs[iref], &fileName)))
						{
							tag_info_p fileInfo = tag_info_new(allRefs[iref]);
							hashtable_p fileProps = snapshot_load_props(fileInfo);
							if(fileProps != NULL)
							{
								//export dataset properties
								for(iprop=0; iprop<fileProps->count; iprop++)
								{
									//DBGLOG("==>Exporing '%s@%s'", fileName, fileProps->kvps[iprop].key);
									_snprintf_s(buff, sizeof(buff), SS_MAXLLEN, "%s@%s:%s\n", fileName, fileProps->kvps[iprop].key, (const char *)(fileProps->kvps[iprop].value));
									fputs(buff, file);
								}
							}
							ht_free(fileProps);
							tag_info_free(fileInfo);
							TC_FREE(fileName);
						}
					}
					TC_FREE(allRefs);
				}
				//
				fclose(file);
				//upload new file
				if(ITK_EVAL(IMF_fmsfile_import(tmpFile, NULL, SS_TEXT, &newFileTag, &fileDescriptor)))
				{
					char *path = NULL;
					if(ITK_EVAL(IMF_ask_file_pathname2(newFileTag, SS_MACHINE_TYPE, &path)))
					{
						DBGLOG("Imported:'%s'", path);
						TC_FREE(path);
					}
				}
				return snapshot_set_prop(tagInfo->tagId, tagInfo->className, newFileTag);
			}
			else
			{
				ERRLOG("Failed to create: %s", tmpFile);
			}
		}
	}
	else
	{
		DBGLOG(TAG_INFO_FMT":Snapshot is not enabled", TAG_INFO_ARGS(tagInfo));
	}
	return FALSE;
}
BOOL snapshot_clean(tag_info_ro tagInfo)
{
	if(SNAPSHOT_IS_ENABLED(tagInfo))
	{
		DBGLOG(TAG_INFO_FMT":Cleaning Snapshot", TAG_INFO_ARGS(tagInfo));
		if(tagInfo->rootType == type_dataset)
		{
			//set property of previous revison to NULL
			tag_t prevRev = NULLTAG;
			int irev = 0;
			if(ITK_EVAL(AE_ask_dataset_latest_rev(tagInfo->tagId, &prevRev))
				&& prevRev != NULLTAG)
			{
				irev++;
				DBGLOG("Updating Rev-%d", irev);
				while(snapshot_set_prop(prevRev, tagInfo->className, NULLTAG))
				{
					if(ITK_EVAL(AE_ask_dataset_prev_rev(prevRev, &prevRev))
						&& prevRev != NULLTAG)
					{
						irev++;
						DBGLOG("Updating Rev-%d", irev);
						continue;
					}
					break;
				}
			}
			DBGLOG("Updating Rev-0");
		}
		return snapshot_set_prop(tagInfo->tagId, tagInfo->className, NULLTAG);
	}
	else
	{
		DBGLOG(TAG_INFO_FMT":Snapshot is not enabled", TAG_INFO_ARGS(tagInfo));
		return FALSE;
	}
}
#define COMPARE_PROPAGATION 2
#define COMPARE_DIFFERENT 1
#define COMPARE_SAME 0
#if TC_MAJOR_VERSION==10000
#define PROPERTY_CONST_PROPAGATION "Fnd0SecurityPropagationEnabled"
#define PROPERTY_CONST_PROPAGATION_DEFAULT "false"
#elif TC_MAJOR_VERSION>=11000
#define PROPERTY_CONST_PROPAGATION "Fnd0PropagationGroup"
#define PROPERTY_CONST_PROPAGATION_DEFAULT "No Group"
#endif
BOOL snapshot_compare_properties(tag_info_ro tagInfo, COMPARE_RESULT *result)
{
	if(SNAPSHOT_IS_ENABLED(tagInfo))
	{
		hashtable_p oldProps = snapshot_load(tagInfo);
		DBGLOG(TAG_INFO_FMT":Comparing protect properties with snapshot", TAG_INFO_ARGS(tagInfo));
		if(oldProps != NULL)
		{
			int iprop;
			int nchanged = 0;
			int hasPropagationPropertyChange = 0;
			kvl_p keys = pref_get_protect_attributes();
			hashtable_p newProps = snapshot_load_props(tagInfo);
			for(iprop=0; iprop < keys->count; iprop++)
			{
				BOOL valueChanged = FALSE;
				const char *key = keys->keys[iprop];
				const char *oldVal = ht_get_chars(oldProps, key);
				const char *newVal = ht_get_chars(newProps, key);
				if(oldVal != NULL)
				{
					if(newVal == NULL)
					{
						DBGLOG("[%d/%d]'%s':Removed '%s'", iprop+1, keys->count, key, oldVal);
						valueChanged = TRUE;
					}
					else if(strcmp(oldVal, newVal)!=0)
					{
						DBGLOG("[%d/%d]'%s':Updated(Old='%s' New='%s')", iprop+1, keys->count, key, oldVal, newVal);
						valueChanged = TRUE;
					}
				}
				else
				{
					if(newVal != NULL)
					{
						DBGLOG("[%d/%d]'%s':Added '%s'", iprop+1, keys->count, key, newVal);
						valueChanged = TRUE;
					}
				}
				if(valueChanged)
				{
					nchanged++;
					if(!hasPropagationPropertyChange)
					{
						tag_t classId = tagInfo->classId;
						char *className = TC_DUP(tagInfo->className);
						while(className != NULL)
						{
							char *pConstantValue = NULL;
							if(ITK_EVAL(CONSTANTS_get_property_constant_value(PROPERTY_CONST_PROPAGATION, className, key, &pConstantValue)))
							{
								NEED_FREE(pConstantValue);
								DBGLOG("%s.%s."PROPERTY_CONST_PROPAGATION"=%s", className, key, pConstantValue);
								hasPropagationPropertyChange = pConstantValue != NULL && _stricmp(pConstantValue, PROPERTY_CONST_PROPAGATION_DEFAULT) != 0 ? COMPARE_PROPAGATION : 0;
								TC_FREE(pConstantValue);
								TC_FREE(className);//break while
							}
							else
							{
								//search value from super class
								tag_t superClassId = NULLTAG;
								char *superClassName = NULL;
								int nsuper;
								tag_t *superClasses = NULL;
								if(ITK_EVAL(POM_superclasses_of_class(classId, &nsuper, &superClasses)))
								{
									NEED_FREE(superClasses);
									if(nsuper > 0)
									{
										superClassId = superClasses[0];
										if(ITK_EVAL(POM_name_of_class(superClassId, &superClassName)))
										{
											DBGLOG("%s(%u) has %d super classes.First=%s(%u)", className, classId, nsuper, superClassName, superClassId);
										}
										else
										{
											ERRLOG("%s(%u) has %d super classes([0]=%u)", className, classId, nsuper, superClassId);
										}
									}
									else
									{
										DBGLOG("%s(%u) has 0 super classes", className, classId);
									}
									TC_FREE(superClasses);
								}
								else
								{
									ERRLOG("Failed to get super class of %s(%u)", className, classId);
								}
								classId = superClassId;
								TC_FREE(className);
								className = superClassName;
							}
						}
					}
				}
				else
				{
					DBGLOG("[%d/%d]'%s':Unchanged('%s')", iprop+1, keys->count, key, oldVal);
				}
			}
			kvl_free(keys);
			ht_free(oldProps);
			ht_free(newProps);
			DBGLOG(TAG_INFO_FMT":Found %d property changes(Propagation=%d)", TAG_INFO_ARGS(tagInfo), nchanged, hasPropagationPropertyChange);
			*result = nchanged > 0 ? (COMPARE_DIFFERENT | hasPropagationPropertyChange): COMPARE_SAME;
			return TRUE;
		}
		else
		{
			DBGLOG(TAG_INFO_FMT":Snapshot doesn't exist", TAG_INFO_ARGS(tagInfo));
		}
	}
	else
	{
		DBGLOG(TAG_INFO_FMT":Snapshot is not enabled", TAG_INFO_ARGS(tagInfo));
	}
	return FALSE;
}
BOOL snapshot_compare_files(tag_info_ro dsInfo, COMPARE_RESULT *result)
{
	if(dsInfo!= NULL
		&& dsInfo->rootType == type_dataset
		&& SNAPSHOT_IS_ENABLED(dsInfo))
	{
		hashtable_p oldProps = snapshot_load(dsInfo);
		if(oldProps != NULL)
		{
			int iprop;
			int allRefCnt;
			tag_t *allRefs = NULL;
			int nchanges = 0;
			hashtable_p newFileProps = ht_create(10);
			//get current file properties
			if(ITK_EVAL(AE_ask_dataset_named_refs(dsInfo->tagId, &allRefCnt, &allRefs)))
			{
				int iref = 0;
				for(iref=0; iref<allRefCnt; iref++)
				{
					char *fileName = NULL;
					if(ITK_EVAL(IMF_ask_original_file_name2(allRefs[iref], &fileName)))
					{
						tag_info_p fileInfo = tag_info_new(allRefs[iref]);
						hashtable_p fileProps = snapshot_load_props(fileInfo);
						if(fileProps != NULL)
						{
							char propKey[MAX_PATH];
							//export dataset properties
							for(iprop=0; iprop<fileProps->count; iprop++)
							{
								const char *oldVal = NULL;
								const char *newVal = (const char *)fileProps->kvps[iprop].value;
								sprintf_s(propKey, MAX_PATH, "%s@%s", fileName, fileProps->kvps[iprop].key);
								if(ht_tryget_chars(oldProps, propKey, &oldVal))
								{
									if(strcmp(oldVal, (const char *)fileProps->kvps[iprop].value)!=0)
									{
										DBGLOG("'%s':Updated", propKey);
										DBGLOG("==>Old:'%s'", oldVal);
										DBGLOG("==>New:'%s'", newVal);
										nchanges++;
									}
								}
								else
								{
									DBGLOG("'%s':Added '%s'", propKey, newVal);
										nchanges++;
								}
								ht_set_chars(newFileProps, propKey, newVal);
							}
						}
						ht_free(fileProps);
						tag_info_free(fileInfo);
						TC_FREE(fileName);
					}
				}
				TC_FREE(allRefs);
			}
			for(iprop=0; iprop<oldProps->count; iprop++)
			{
				const char *oldKey = oldProps->kvps[iprop].key;
				if(strstr(oldKey, "@") != NULL)
				{
					//this is a key of file property
					if(!ht_contains_key(newFileProps, oldKey))
					{
						DBGLOG("'%s':Removed '%s'", oldKey, oldProps->kvps[iprop].value);
											nchanges++;
					}
				}
			}
			ht_free(newFileProps);
			ht_free(oldProps);
			DBGLOG(TAG_INFO_FMT":Found %d property changes in %d named reference(s)", nchanges, allRefCnt, TAG_INFO_ARGS(dsInfo));
			*result = nchanges > 0 ? COMPARE_DIFFERENT : COMPARE_SAME;
			return TRUE;
		}
		else
		{
			DBGLOG(TAG_INFO_FMT":Snapshot doesn't exist", TAG_INFO_ARGS(dsInfo));
		}
	}
	else
	{
		DBGLOG(TAG_INFO_FMT":Snapshot is not enabled", TAG_INFO_ARGS(dsInfo));
	}
	return FALSE;
}
BOOL imanfile_validate_name(tag_info_ro fileInfo, char **outputName, BOOL *outputIsProtected)
{
	char *displayName = NULL;
	BOOL ret = FALSE;
	if(ITK_EVAL(IMF_ask_original_file_name2(fileInfo->tagId, &displayName)))
	{
		BOOL isProtected = FALSE;
		NEED_FREE(displayName);
		if(imanfile_is_protected_fsc(fileInfo, &isProtected))
		{
			char newName[MAX_PATH] = "";
			BOOL hasNxlExt = string_ends_with(displayName, NXL_SUFFIX, FALSE);
			//set output
			*outputIsProtected = isProtected;
			ret = TRUE;
			//validate name
			if(hasNxlExt && !isProtected)
			{
				//file is not protected but name contains .nxl ext
				//remove .nxl extension
				strncpy_s(newName, MAX_PATH, displayName, strlen(displayName)-strlen(NXL_SUFFIX));
				imanfile_rename(fileInfo, newName);
				//TBD:how about the reference name?
				DBGLOG(TAG_INFO_FMT":IsProtected=%d OldName='%s' NewName='%s'", TAG_INFO_ARGS(fileInfo), isProtected, displayName, *outputName);
			}
			else if(!hasNxlExt && isProtected)
			{
				if((get_solution_info())->type==solution_inMotion)
				{
					DBGLOG(TAG_INFO_FMT":IsProtected=%d DisplayName='%s' Skip appending .nxl extension in InMotion mode", TAG_INFO_ARGS(fileInfo), isProtected, displayName);
				}
				else
				{
					//file is protected but name doesn't contain .nxl ext
					// append .nxl to file display name 
					//the server side request handler will use this display name as a flag, to avoid checking content again
					sprintf_s(newName, MAX_PATH, "%s"NXL_SUFFIX, displayName);
					imanfile_rename(fileInfo, newName);
					DBGLOG(TAG_INFO_FMT":IsProtected=%d OldName='%s' NewName='%s'", TAG_INFO_ARGS(fileInfo), isProtected, displayName, newName);
				}
			}
			else
			{
				DBGLOG(TAG_INFO_FMT":IsProtected=%d DisplayName='%s'", TAG_INFO_ARGS(fileInfo), isProtected, displayName);
			}
			//set new name
			if(!string_isNullOrSpace(newName))
			{
				TC_FREE(displayName);
				displayName = TC_DUP(newName);
			}
		}
		if(outputName != NULL)
		{
			*outputName = displayName;
		}
	}
	return ret;
}

BOOL dataset_find_nr_info(tag_info_ro dsInfo, const char *rName, int rType, const char *fileName, tag_info_p *fileInfo) {
	int nrefs = 0;
	*fileInfo = NULL;
	if (dsInfo != NULL
		&& rName != NULL
		&& ITK_EVAL(AE_ask_dataset_ref_count(dsInfo->tagId, &nrefs))
		&& nrefs > 0)
	{
		int iref;
		for (iref = 0; iref < nrefs; iref++)
		{
			char *refName;
			AE_reference_type_t refType;
			tag_t refTag;
			if (ITK_EVAL(AE_find_dataset_named_ref2(dsInfo->tagId, iref, &refName, &refType, &refTag)))
			{
				NEED_FREE(refName);
				if ((rType == 0 || rType == refType)
					&& strcmp(rName, refName)==0)
				{
					tag_info_p fInfo = tag_info_new(refTag);
					char *displayName = NULL;
					ITK_CALL(IMF_ask_original_file_name2(fInfo->tagId, &displayName));
					NEED_FREE(displayName);
					if (displayName != NULL
						&& *fileInfo == NULL
						&& (fileName == NULL || strcmp(displayName, fileName) == 0)) {
						*fileInfo = fInfo;
						DBGLOG(TAG_INFO_FMT":NR[%d/%d]" TAG_INFO_FMT ":refName='%s' refType=%d DisplayName='%s'(Found)",
							TAG_INFO_ARGS(dsInfo), iref + 1, nrefs, TAG_INFO_ARGS(fInfo), refName, refType, displayName);
					}
					else
					{
						DBGLOG(TAG_INFO_FMT":NR[%d/%d]" TAG_INFO_FMT ":refName='%s' refType=%d DisplayName='%s'",
							TAG_INFO_ARGS(dsInfo), iref + 1, nrefs, TAG_INFO_ARGS(fInfo), refName, refType, displayName);
						tag_info_free(fInfo);
					}
					
					TC_FREE(displayName);
				}
				TC_FREE(refName);
			}
		}
	}
	return *fileInfo != NULL;
}

int dataset_list_ref_infos(tag_info_ro dsInfo, BOOL includeAll, tag_info_p **nrTagInfos)
{
	int nrefs = 0;
	int returnedRefs = 0;
	if(dsInfo != NULL
		&& ITK_EVAL(AE_ask_dataset_ref_count(dsInfo->tagId, &nrefs))
		&& nrefs > 0)
	{
		int iref;
		string_list_p nrBlacklist = NULL;
		if(!includeAll)
		{
			nrBlacklist = pref_nr_blacklist();
		}
		//initialize output arrays
		TC_ALLOCN(*nrTagInfos, tag_info_p, nrefs);
		for(iref = 0; iref < nrefs; iref++)
		{
			char *refName;
			AE_reference_type_t refType;
			tag_t refTag;
			if(ITK_EVAL(AE_find_dataset_named_ref2(dsInfo->tagId, iref, &refName, &refType, &refTag)))
			{
				BOOL inBlacklist = string_list_index_of(nrBlacklist, refName) >= 0;
				NEED_FREE(refName);
				if(!inBlacklist)
				{
					tag_info_p fileInfo = tag_info_new(refTag);
					DBGLOG(TAG_INFO_FMT":NR[%d/%d]-{refTag=%u refName='%s' refType=%d}-"TAG_INFO_FMT,
						TAG_INFO_ARGS(dsInfo), iref+1, nrefs, refTag, refName, refType, TAG_INFO_ARGS(fileInfo));
					(*nrTagInfos)[returnedRefs++] = fileInfo;
				}
				else
				{
					DBGLOG(TAG_INFO_FMT":NR[%d/%d]-{refTag=%u refName='%s' refType=%d}:InBlackList=%d", TAG_INFO_ARGS(dsInfo), iref+1, nrefs, refTag, refName, refType, inBlacklist);
				}
				TC_FREE(refName);
			}
		}
		if(returnedRefs == 0)
		{
			//initialize output arrays
			TC_FREE(*nrTagInfos);
		}
		string_list_free(nrBlacklist);
	}
	return returnedRefs;
}

void EnumerateDatasetReferences(tag_t dsTag, EnumNRCallback cb, void* cbData) {
	//list references
	int nrefs = 0;
	if (dsTag != NULLTAG
		&& ITK_EVAL(AE_ask_dataset_ref_count(dsTag, &nrefs))
		&& nrefs > 0)
	{
		int iref;
		string_list_p nrBlacklist = pref_nr_blacklist();
		for (iref = 0; iref < nrefs; iref++)
		{
			char* refName;
			AE_reference_type_t refType;
			tag_t refTag;
			EnumNRCallbackParams_t params;
			if (ITK_EVAL(AE_find_dataset_named_ref2(dsTag, iref, &refName, &refType, &refTag)))
			{
				BOOL skip = FALSE;
				NEED_FREE(refName);
				params.refName = refName;
				params.refType = refType;
				params.refTag = refTag;
				params.isblacklisted = string_list_index_of(nrBlacklist, refName) >= 0;
				params.cbData = cbData;
				DBGLOG("NR[%d/%d]-{refTag=%u refName='%s' refType=%d blacklisted=%d}",
					iref + 1, nrefs, refTag, refName, refType, params.isblacklisted);
				if(cb != NULL)
				{
					skip = cb(&params);
				}
				TC_FREE(refName);
				if (skip) {
					break;
				}
			}
		}
	}
	else
	{
		DBGLOG("%u:has 0 named references", dsTag);
	}
}
void EnumerateItemRevDatasets(tag_t revTag, EnumerateCallback cb, void *cbData)
{
	int cntSecObj;
	tag_t* secObjs;
	//get children objects from ItemRevision
	if (revTag != NULLTAG
		&& ITK_EVAL(GRM_list_secondary_objects_only(revTag, NULLTAG, &cntSecObj, &secObjs))
		&& cntSecObj > 0)
	{
		int iSecObj;
		BOOL skip = false;
		for (iSecObj = 0; iSecObj < cntSecObj; iSecObj++)
		{
			EnumerateCallbackParams_t params;
			params.itemTag = secObjs[iSecObj];
			params.cbData = cbData;
			DBGLOG("SecObj[%d/%d]-%u"
				, iSecObj + 1, cntSecObj, secObjs[iSecObj]);
			if (cb != NULL) {
				skip = cb(&params);
			}
			
			if (skip)
				break;
		}//End for(iSecObj = 0; iSecObj < cntSecObj; iSecObj++)

		TC_FREE(secObjs);
	}
	else
	{
		DBGLOG("%u:has 0 Secondary Objects", revTag);
	}

}