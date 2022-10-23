/*=============================================================================
                 Copyright (c) 2015 Nextlabs.
                   Unpublished - All rights reserved

   THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY THE UNITED STATES
  COPYRIGHT LAWS AND IS CONSIDERED A TRADE SECRET BELONGING TO THE COPYRIGHT
                                   HOLDER.
===============================================================================
File description:

    File   : nxl_auto_protect.h 
    Module : NXL-auto-protect header file

===============================================================================

$HISTORY$
Date            Name				Description of Change 
2014-05-27      Chenwei Low         Initial Version/POC
2015-05-27		Gavin Chen			Version 1.5
=============================================================================*/
#include <tccore/grm.h> //GRM_list_secondary_objects_only(); GRM_list_primary_objects_only()
#include <tc/preferences.h> //PREF_ask_char_values()
#include <tccore/aom_prop.h> //AOM_ask_value_tags()
//NextLabs
#include "dce_mem.h"
#include "dce_log.h"
#include "utils_string.h"
#include "utils_time.h"
#include "dce_evaluator.h"
#include "dce_translator.h"
#include <key_value_list.h>

//
//Defines
//
#define ARG_KEY_DESIRED_DATA_SET_TYPE "DatasetTypeToProtect"//To be deprecated in Ver3.0
#define ARG_KEY_SCHEDULED_TIME "ScheduledTime"	//To be deprecated in Ver3.0
#define ARG_KEY_DATASET_TYPES "dataset_types"	//NOTE: replace for DatasetTypeToProtect new format that follows COTS handlers
#define ARG_KEY_RUN_AT_DATETIME "datetime"	//Replacement for ScheduledTime
#define ARG_KEY_RUN_AT_TIME "time"
#define ARG_KEY_RELATION_TYPES "relation_types"
#define ARG_KEY_PROTECT_ASSEMBLY "protect_assembly"	
//NOTE:In 3.5, we introduce remove protection in workflow, so protect_assembly is renamed to process_assembly, support for protect_assembly may be ceased in any future release
#define ARG_KEY_PROCESS_ASSEMBLY "process_assembly"
#define ARG_KEY_SKIP_NXL_DATASETS "skip_nxl_datasets"
#define ARG_OPERATION_KEY "operation"
#define ARG_OPERATION_UNPROTECT "remove-protect"
#define ARG_VALUES_SPLITTERS ","

#define PROP_CHILD_ITEM_REVISION "ps_children"

//
//Action Handler Arguments Module
//

//
//Prototypes
//

void process_item_revision(tag_info_ro revInfo, wf_args_p wfargs);
void process_dataset(tag_info_ro dsInfo, wf_args_p wfargs, tag_info_ro revInfo);

wf_args_p load_workflow_arguments(EPM_action_message_t msg)
{
	char *argStr;
	wf_args_p wfargs = WF_ARGS_new();

	TC_init_argument_list(msg.arguments);
	DBGLOG("Loading %d Arguments...",TC_number_of_arguments(msg.arguments));

	while((argStr = TC_next_argument(msg.arguments))!=0)
	{
		char *argKey,*argValue;
		if(ITK_EVAL(ITK_ask_argument_named_value(argStr, &argKey, &argValue)))
		{
			NEED_FREE(argKey);
			NEED_FREE(argValue);
			DBGLOG("==>'%s' : '%s'", argKey, argValue);
			if(string_isNullOrSpace(argKey))
				goto NEXT_ARG;
			if(wfargs->datasetTypes->count == 0	//data_type is not initialized
				&&(strcmp(argKey, ARG_KEY_DESIRED_DATA_SET_TYPE) == 0
					||strcmp(argKey, ARG_KEY_DATASET_TYPES) == 0))
			{
				string_split(argValue, ARG_VALUES_SPLITTERS, wfargs->datasetTypes);
				DBGLOG("Argument:Desire %d DataSet Type(s) is %s", wfargs->datasetTypes->count, argValue);
			}
			else if(wfargs->scheduledTime <= 0	//scheduleTime is not initialized
				&&(strcmp(argKey, ARG_KEY_SCHEDULED_TIME) == 0
					||strcmp(argKey, ARG_KEY_RUN_AT_DATETIME) == 0))
			{
				int year,month,day,hour,min;
				float tz = 99;//set it as an invalid timezone number
				struct tm tm;
				sscanf_s(argValue,"%d/%d/%d %d:%d UTC%f", &month, &day, &year, &hour, &min, &tz);
				tm.tm_year = year - 1900;
				tm.tm_mon = month - 1;
				tm.tm_mday = day;
				tm.tm_hour = hour;
				tm.tm_min = min;
				tm.tm_sec = 0;
				tm.tm_isdst = -1;
				wfargs->scheduledTime = time_convert(tm, tz);
				DBGLOG("Argument:ScheduledTime='%s'", time_tostring(wfargs->scheduledTime));
			}
			else if(wfargs->scheduledTime <= 0
				&& strcmp(argKey, ARG_KEY_RUN_AT_TIME) == 0)
			{
				int hour,min;
				float tz = 99;//set it as an invalid timezone number
				struct tm tm, *curTimeStruct;
				time_t scheduledTime;
				time_t curTime = time(NULL);
				curTimeStruct = localtime(&curTime);
				sscanf_s(argValue, "%d:%d UTC%f", &hour, &min, &tz);
				tm.tm_year = curTimeStruct->tm_year;
				tm.tm_mon = curTimeStruct->tm_mon;
				tm.tm_mday = curTimeStruct->tm_mday;
				if(hour >= 0 && hour < 24)
					tm.tm_hour = hour;
				else
					tm.tm_hour = 0;
				if(min >= 0 && min < 60)
					tm.tm_min = min;
				else
					tm.tm_min = 0;
				tm.tm_sec = 0;
				tm.tm_isdst = -1;
				scheduledTime = time_convert(tm, tz);
#define SECONDS_OF_ONE_DAY 24*60*60
				while(scheduledTime < curTime)
				{
					//schedule to next day
					scheduledTime += SECONDS_OF_ONE_DAY;
				}
				while(scheduledTime > (curTime + SECONDS_OF_ONE_DAY))
				{
					scheduledTime -= SECONDS_OF_ONE_DAY;
				}
				wfargs->scheduledTime = scheduledTime;
				DBGLOG("Argument:ScheduledTime='%s'", time_tostring(scheduledTime));
			}
			else if(wfargs->relationTypes->count <= 0
				&& strcmp(argKey, ARG_KEY_RELATION_TYPES) == 0)
			{
				string_list_p inputNames = string_list_new();
				if((string_split(argValue, ARG_VALUES_SPLITTERS, inputNames)) > 0)
				{
					find_relation_types(inputNames->items, inputNames->count, wfargs->relationTypes, wfargs->relationNames);
					DBGLOG("Argument: %d relation types to be protected", wfargs->relationTypes->count);
				}
				string_list_free(inputNames);
			}
			else if(wfargs->process_assembly == 0	//protect_assembly has not been initialized yet
				&& (strcmp(argKey, ARG_KEY_PROTECT_ASSEMBLY) == 0
					||strcmp(argKey, ARG_KEY_PROCESS_ASSEMBLY) == 0))
			{
				wfargs->process_assembly = _stricmp(argValue, "TRUE") == 0 ? 1 : -1;
				DBGLOG("Argument: ProcessAssembly=%d", wfargs->process_assembly);
			}
			else if(wfargs->skip_nxl_datasets == 0	//not initialized yet
				&& _stricmp(argKey, ARG_KEY_SKIP_NXL_DATASETS)==0)
			{
				wfargs->skip_nxl_datasets = _stricmp(argValue, "TRUE") == 0 ? 1 : -1;
				DBGLOG("Argument: "ARG_KEY_SKIP_NXL_DATASETS"=%d", wfargs->skip_nxl_datasets);
			}
			else if(wfargs->operation == wfop_undefined
				&& _stricmp(argKey, ARG_OPERATION_KEY) == 0
				&& _stricmp(argValue, ARG_OPERATION_UNPROTECT) == 0)
			{
				wfargs->operation = wfop_unprotect;
				DBGLOG("Argument: "ARG_OPERATION_KEY"=%d", wfargs->operation);
			}
			//
NEXT_ARG:
			TC_FREE(argKey);
			TC_FREE(argValue);
		}
		else
		{
			ERRLOG("==>Non-recognized argument:%s",argStr);
		}
	}
	//validate workflow args
	//set default operation as protect
	if(wfargs->operation == wfop_undefined)
		wfargs->operation = wfop_protect;
	//set default relationType as all relations
	if(wfargs->relationTypes->count==0)
	{
		//if no specific relation type is defined, load all relations by NULLTAG
		
		int nNames = -1;
		char **names = NULL;
		if(ITK_EVAL(PREF_ask_char_values(PREF_RELATION_FILTER, &nNames, &names))
			&& nNames > 0)
		{
			NEED_FREE(names);
			find_relation_types(names, nNames, wfargs->relationTypes, wfargs->relationNames);
			DBGLOG("Argument:'%s' is not defined, %d/%d relation types to be protected as per Preference-'%s'",
				ARG_KEY_RELATION_TYPES,
				wfargs->relationTypes->count,
				nNames,
				PREF_RELATION_FILTER);
			TC_FREE(names);
		}
		else
		{
			DBGLOG("Argument:Both Argument-'%s' and Preference-'%s' are not defined, load all relations",
				ARG_KEY_RELATION_TYPES,
				PREF_RELATION_FILTER);
			TAG_LIST_add(wfargs->relationTypes, NULLTAG);
		}
	}
	return wfargs;
}
//Implementations
int nxl_auto_protect_handler(EPM_action_message_t msg) 
{
	//
	wf_args_p wfargs = NULL;
	tag_t task = msg.task;
	tag_t rootTask;
	char *taskName, *rootTaskName;
	
	TC_HANDLER_START;
	//task
	ITK_CALL(EPM_ask_name2(task, &taskName));
	NEED_FREE(taskName);

	//root task
	ITK_CALL(EPM_ask_root_task(task, &rootTask));
	ITK_CALL(EPM_ask_name2(rootTask, &rootTaskName));
	NEED_FREE(rootTaskName);
	DBGLOG("******Action Handler is executing for Task(%s/%s)Action=%d ProposedState=%d", rootTaskName, taskName, msg.action, msg.proposed_state);

	if(msg.proposed_state == EPM_pending)
	{
		DBGLOG("Skipping pending action");
		goto CLEAN;
	}

	wfargs = load_workflow_arguments(msg);
	//cache workflow name
	wfargs->workflowName = rootTaskName;

	//checking attachements
	{
		int *attTypeList;			

		if(ITK_EVAL(EPM_ask_all_attachments(rootTask, &(wfargs->attachments->count), &(wfargs->attachments->tags), &attTypeList)))
		{
			int iAtt;
			NEED_FREE(wfargs->attachments->tags);
			NEED_FREE(attTypeList);
			DBGLOG("Initializing %d Attachments...", wfargs->attachments->count);
			for(iAtt = 0; iAtt < wfargs->attachments->count; iAtt++)
			{
				tag_info_p attInfo  = tag_info_new(wfargs->attachments->tags[iAtt]);
				tag_info_ro parentInfo = NULL;
				int attType = attTypeList[iAtt];
				//only check attachment whose type is EPM_target_attachment
				if(attType != EPM_target_attachment)
				{
					DBGLOG("Attachment Objects[%d/%d]-"TAG_INFO_FMT" AttType=%d EPM_target_attachment=%d"
						, iAtt+1, wfargs->attachments->count, TAG_INFO_ARGS(attInfo), attType, EPM_target_attachment);
					goto NEXT_ATT;
				}
				//check if the attachment can be the descendant of ItemRevision
				switch(attInfo->rootType)
				{
					case type_dataset:
						//this is a dataset from attachment
						parentInfo = tag_info_get_parent(attInfo);
						//skip processing if its parent item revision is also attached
						if(parentInfo != NULL
							&& TAG_LIST_contains(wfargs->attachments, parentInfo->tagId))
						{
							DBGLOG("Attachment Objects[%d/%d]-"TAG_INFO_FMT":Skip processing this dataset because its paren("TAG_INFO_FMT") is also included in the attachments"
								, iAtt+1, wfargs->attachments->count, TAG_INFO_ARGS(attInfo), TAG_INFO_ARGS(parentInfo));
							break;
						}
						DBGLOG("Attachment Objects[%d/%d]-"TAG_INFO_FMT
							, iAtt+1, wfargs->attachments->count, TAG_INFO_ARGS(attInfo));
						CALL_DBG(process_dataset(attInfo, wfargs, parentInfo));
						break;
					case type_item_revision:
						DBGLOG("Attachment Objects[%d/%d]-"TAG_INFO_FMT
							, iAtt+1, wfargs->attachments->count, TAG_INFO_ARGS(attInfo));
						CALL_DBG(process_item_revision(attInfo, wfargs));
						break;
					default:
						DBGLOG("Attachment Objects[%d/%d]-"TAG_INFO_FMT":Unhandled Object Type"
							, iAtt+1, wfargs->attachments->count, TAG_INFO_ARGS(attInfo));
						break;
				}
NEXT_ATT:
				CALL_DTL(tag_info_free(attInfo));
			}
			TC_FREE(attTypeList);
		}//EPM_ask_all_attachments
	}//end attachments


	DBGLOG("Processed %d Dataset(s) %d ItemRevision(s) and sent %d Translation Requests", 
		wfargs->datasets->count, 
		wfargs->itemRevisions->count,
		wfargs->translations->count);

CLEAN:
	WF_ARGS_free(wfargs);
	DBGLOG("******************%s/%s Finished", rootTaskName, taskName);
	TC_FREE(taskName);
	TC_FREE(rootTaskName);
	TC_HANDLER_END;
	return 0;
}

void process_item_revision(tag_info_ro revInfo, wf_args_p wfargs)
{
	int iRelation;
	if(revInfo->rootType != type_item_revision)
	{
		DBGLOG(TAG_INFO_FMT" is not an ItemRevision", TAG_INFO_ARGS(revInfo));
		return;
	}
	//check if the ItemRevision has been processed
	if(TAG_LIST_contains(wfargs->itemRevisions, revInfo->tagId))
	{
		DBGLOG(TAG_INFO_FMT" has been processed!", TAG_INFO_ARGS(revInfo));
		return;
	}
	//
	//checking ItemRevision
	//
	DBGLOG(TAG_INFO_FMT":Searching Secondary Object by relationTypes(%d)", TAG_INFO_ARGS(revInfo), wfargs->relationTypes->count);
	for(iRelation = 0; iRelation < wfargs->relationTypes->count; iRelation++)
	{
		int nGrmObj;
		GRM_relation_t *listGrmObj;
		tag_t relationType = wfargs->relationTypes->tags[iRelation];
		const char *relationTypeName = NULL;
		if(relationType != NULLTAG)
		{
			relationTypeName = wfargs->relationNames->items[iRelation];
		}
		//get children objects from ItemRevision
		if(ITK_EVAL(GRM_list_secondary_objects(revInfo->tagId, relationType, &nGrmObj, &listGrmObj))
			&& nGrmObj > 0)
		{
			int iGrmObj;
			NEED_FREE(listGrmObj);
			for(iGrmObj = 0; iGrmObj < nGrmObj; iGrmObj++)
			{
				GRM_relation_t relation = listGrmObj[iGrmObj];
				tag_info_p tagInfo = tag_info_new(relation.secondary);
				if(relationType == NULLTAG)
				{
					char *relName = NULL;
					if(ITK_EVAL(TCTYPE_ask_name2(relation.relation_type, &relName)))
					{
						NEED_FREE(relName);
					}
					DBGLOG(TAG_INFO_FMT":SecondaryObject[%d/%d](Relation=%d'%s')-"TAG_INFO_FMT,
						TAG_INFO_ARGS(revInfo), iGrmObj+1, nGrmObj, relation.relation_type, relName, TAG_INFO_ARGS(tagInfo));
					TC_FREE(relName);
				}
				else
				{
					DBGLOG(TAG_INFO_FMT":Relation[%d/%d](%d'%s'):SecondaryObject[%d/%d]-"TAG_INFO_FMT,
						TAG_INFO_ARGS(revInfo), iRelation+1, wfargs->relationTypes->count, relationType, relationTypeName, iGrmObj+1, nGrmObj, TAG_INFO_ARGS(tagInfo));
				}
				
				if(tagInfo->rootType==type_dataset)
				{
					//proces dataset
					CALL_DBG(process_dataset(tagInfo, wfargs, revInfo));
				}
				tag_info_free(tagInfo);
			}//End for(iGrmObj = 0; iGrmObj < nGrmObj; iGrmObj++)

			TC_FREE(listGrmObj);
		}//if(GRM_list_secondary_objects(revInfo->tagId, relationType, &nGrmObj, &listGrmObj))
		else
		{
			DBGLOG(TAG_INFO_FMT":Relation[%d/%d](%d'%s'):has 0 secondary object",
				TAG_INFO_ARGS(revInfo), iRelation+1, wfargs->relationTypes->count, relationType, relationTypeName);
		}
	}//for(iRelation = 0; iRelation < wfargs->relationTypes->count; iRelation++)
	
	if(wfargs->process_assembly > 0)
	{
		//find View property
		int num;
		tag_t *tags;
		if(ITK_EVAL(AOM_ask_value_tags(revInfo->tagId, PROP_CHILD_ITEM_REVISION, &num, &tags))
			&& num > 0)
		{
			int i;
			NEED_FREE(tags);
			for(i=0; i<num; i++)
			{
				tag_info_p viewInfo = tag_info_new(tags[i]);
				DBGLOG(TAG_INFO_FMT":BOMViewRevision[%d/%d]-"TAG_INFO_FMT, TAG_INFO_ARGS(revInfo), i+1, num, TAG_INFO_ARGS(viewInfo));
				if(viewInfo->rootType == type_item_revision)
				{
					CALL_DBG(process_item_revision(viewInfo, wfargs));
				}
				tag_info_free(viewInfo);
			}
			TC_FREE(tags);
		}
		else
		{
			DBGLOG(TAG_INFO_FMT" has 0 BOMViewRevision", TAG_INFO_ARGS(revInfo));
		}
	}
	//set this ItemRevision as processed
	TAG_LIST_add(wfargs->itemRevisions, revInfo->tagId);
}
void process_dataset(tag_info_ro dsInfo, wf_args_p wfargs, tag_info_ro revInfo)
{
	char  *dsType = NULL;

	if(dsInfo->rootType != type_dataset)
	{
		DBGLOG(TAG_INFO_FMT" is not a Dataset!", TAG_INFO_ARGS(dsInfo));
		return;
	}
	//check if the dataset has been processed
	if(TAG_LIST_contains(wfargs->datasets, dsInfo->tagId))
	{
		DBGLOG(TAG_INFO_FMT" has been processed!", TAG_INFO_ARGS(dsInfo));
		return;
	}
	//check if need to skip nxl datasets
	if(wfargs->operation == wfop_protect
		&& wfargs->skip_nxl_datasets > 0
		&& dataset_is_nxl(dsInfo))
	{
		//
		DBGLOG(TAG_INFO_FMT" is Nextlabs Protected Dataset and Skipped protection!", TAG_INFO_ARGS(dsInfo));
		goto VISITED;
	}
	else if(wfargs->operation == wfop_unprotect
		&& !dataset_is_nxl(dsInfo))
	{
		//
		DBGLOG(TAG_INFO_FMT" is NOT Nextlabs Protected Dataset and Skipped unprotect!", TAG_INFO_ARGS(dsInfo));
		goto VISITED;
	}
	if(!ITK_EVAL(WSOM_ask_object_type2(dsInfo->tagId, &dsType)))
		return;
	NEED_FREE(dsType);
	DBGLOG(TAG_INFO_FMT":Object Type(%s) Expected %d types",
		TAG_INFO_ARGS(dsInfo), dsType, wfargs->datasetTypes->count);
	if((wfargs->datasetTypes->count == 0)	//datasetTypes is not defined
		|| (string_list_index_of(wfargs->datasetTypes, dsType) >= 0))//arg for desired dataset type is specified and is equals to this object type
	{
		request_type requestType = request_non;
		kvl_p tagMap = kvl_new(0);
		
		//check if there are valid parent tag
		if(revInfo == NULL)
		{
			revInfo = tag_info_get_parent(dsInfo);
		}

		/*from Ver2.0 the ItemRevision Properties will not be tagged into encrypted file
		//merge parent ItemRevision properties and Dataset properties for evaluation
		if(revInfo != NULL)
		{
			MAP_merge(dsInfo->props, revInfo->props);
			DBGLOG("Got %d Merged Properties for evaluating "TAG_INFO_FMT, dsInfo->props->count, TAG_INFO_ARGS(dsInfo));
		}*/
		//evaluate
		if(wfargs->operation == wfop_protect
			&& evaluate_workflow_protect(dsInfo, revInfo, tagMap) == DO_TRANSLATION)
		{
			requestType = request_protect;
		}
		else if(wfargs->operation == wfop_unprotect
			&& evaluate_workflow_unprotect(wfargs->workflowName, dsInfo, revInfo) == DO_TRANSLATION)
		{
			requestType = request_unprotect;
		}

		//send translation request
		if(requestType != request_non)
		{
			tag_t requestTag;
			CALL_DBG(requestTag = send_translation_request(dsInfo, revInfo, tagMap, wfargs->scheduledTime, requestType, priority_medium, wfargs->workflowName));
			if(requestTag != NULLTAG)
			{
				CALL_DTL(TAG_LIST_add(wfargs->translations, requestTag));
			}
		}
		//FREE
		CALL_DTL(kvl_free(tagMap));
	}
VISITED:
	//mark this dataset as visited
	CALL_DTL(TAG_LIST_add(wfargs->datasets, dsInfo->tagId));

//CLEANUP_PD:
	TC_FREE(dsType);
}

