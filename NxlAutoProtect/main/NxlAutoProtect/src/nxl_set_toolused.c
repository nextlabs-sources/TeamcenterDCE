
/* 
 * @file 
 *
 *   This file contains the implementation for the Extension Nxl3_dataset_post_action
 *
 */
#include <ae/tool.h>
#include <ae/dataset.h>
#include <ae/datasettype.h>
#include <res/res_itk.h>
#include <tccore/aom.h>
#include <tc/preferences.h>
#include <utils_string.h>
//NxlAutoProtect.dll
#include "dce_log.h"
#include "dce_mem.h"
#include "tc_utils.h"
#include <nxl_action_handler.h>

#define ARG_NAME_TOOL_NAME "ToolName"
#define ARG_NAME_DS_PREFIX "DatasetNamePattern"

#define DBGPREF_DATASET_PREX "NXLDBG_DS_PREFIX"

BOOL dataset_set_tool(tag_info_ro dsInfo, BOOL setNxlTool)
{
	BOOL set = FALSE;
	tag_t nxlTool;
	//get nxl tool tag by name
	if(ITK_EVAL(AE_find_tool2(NXL_TOOL_NAME, &nxlTool)))
	{
		tag_t curTool = NULLTAG;
		char *curToolName = NULL;
		tag_t tarTool = NULLTAG;
		char *tarToolName = NULL;
		//get current dataset tool
		ITK_EVAL(AE_ask_dataset_tool(dsInfo->tagId, &curTool));
		if(curTool != nxlTool)
		{
			if(ITK_EVAL(AE_ask_tool_symbol_name2(curTool, &curToolName)))
			{
				NEED_FREE(curToolName);
			}
			if(setNxlTool)
			{
				tarTool = nxlTool;
				tarToolName = TC_DUP(NXL_TOOL_NAME);
			}
		}
		else
		{
			curToolName = TC_DUP(NXL_TOOL_NAME);
			if(!setNxlTool)
			{
				tag_t dsType;
				//get default tool for this dataset
				if(ITK_EVAL(AE_ask_dataset_datasettype(dsInfo->tagId, &dsType)))
				{
					char *dsTypeName;
					ITK_EVAL(AE_ask_datasettype_name2(dsType, &dsTypeName));
					if(ITK_EVAL(AE_ask_datasettype_def_tool(dsType, &tarTool)))
					{
						if(ITK_EVAL(AE_ask_tool_symbol_name2(tarTool, &tarToolName)))
						{
							NEED_FREE(tarToolName);
						}
					}
					DBGLOG(TAG_INFO_FMT"DatasetType=%u-'%s' DefaultTool=%u-'%s'", TAG_INFO_ARGS(dsInfo), dsType, dsTypeName, tarTool, tarToolName);
					TC_FREE(dsTypeName);
				}
			}
		}
		if(tarTool != NULLTAG && tarTool != curTool)
		{
			BOOL locked = FALSE;
			logical modifiable = false;
			ITK_EVAL(POM_modifiable(dsInfo->tagId, &modifiable));
			DBGLOG(TAG_INFO_FMT":modifiable=%d", TAG_INFO_ARGS(dsInfo), modifiable);
			if(!modifiable)
			{
				if(ITK_EVAL(AOM_refresh(dsInfo->tagId, TRUE)))
				{
					locked = TRUE;
				}
			}
			if(ITK_EVAL(AE_set_dataset_tool(dsInfo->tagId, tarTool))
				//use POM_save_instances to save object changes instead of AE_save_myself to avoid trigger the nxl3_dataset_save_post handler
				//don't use AOM_save() which will create a revision on dataset, the following changes will be dropped after cancel checkout
				&& ITK_EVAL(POM_save_instances(1, &(dsInfo->tagId), FALSE)))
			{
				DBGLOG(TAG_INFO_FMT":Changed tool from %u('%s') to %u('%s')", TAG_INFO_ARGS(dsInfo), curTool, curToolName, tarTool, tarToolName);
				set = TRUE;
			}
			else
			{
				ERRLOG(TAG_INFO_FMT":FAILED:CurrentTool=%u('%s') TargetTool=%u('%s')", TAG_INFO_ARGS(dsInfo), curTool, curToolName, tarTool, tarToolName);
			}
			if(locked)
			{
				ITK_EVAL(AOM_refresh(dsInfo->tagId, FALSE));
			}
		}
		else
		{
			DBGLOG(TAG_INFO_FMT"KeepCurrentTool=%u(%s) NxlTool=%u("NXL_TOOL_NAME")", TAG_INFO_ARGS(dsInfo), curTool, curToolName, nxlTool);
		}
		TC_FREE(tarToolName);
		TC_FREE(curToolName);
	}
	return set;
}

ref_status_t dataset_verify_ref_status(tag_info_ro dsInfo, BOOL validateFile, BOOL updateTool)
{
	ref_status_t status = {0};
	int nrefs = 0;
	if(dsInfo != NULL
		&& ITK_EVAL(AE_ask_dataset_ref_count(dsInfo->tagId, &nrefs))
		&& nrefs > 0)
	{
		int iref;
		string_list_p nrBlacklist = pref_nr_blacklist();
		char *dsType = dataset_get_type(dsInfo);
		string_list_p displayNames = string_list_new();
		tag_list_p fileTags = TAG_LIST_new();
		string_list_p fileRefNames = string_list_new();
		tag_list_p fileToBeDeleted = TAG_LIST_new();
		string_list_p refNameToBeDeleted = string_list_new();
		status.nTotal = nrefs;
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
					char *displayName = NULL;
					BOOL isprotected;
					if(validateFile
						&& imanfile_validate_name(fileInfo, &displayName, &isprotected))
					{
						//file validate success
						DBGLOG(TAG_INFO_FMT":NR[%d/%d]-{refTag=%u refName='%s' refType=%d}-"TAG_INFO_FMT":DisplayName='%s' IsProtected=%d"
							, TAG_INFO_ARGS(dsInfo), iref+1, nrefs, refTag, refName, refType, TAG_INFO_ARGS(fileInfo), displayName, isprotected);
					}
					else if(validateFile
						|| ITK_EVAL(IMF_ask_original_file_name2(fileInfo->tagId, &displayName)))
					{
						//decide protection status by file name
						NEED_FREE(displayName);
						DBGLOG(TAG_INFO_FMT":NR[%d/%d]-{refTag=%u refName='%s' refType=%d}-"TAG_INFO_FMT":DisplayName='%s'"
							, TAG_INFO_ARGS(dsInfo), iref+1, nrefs, refTag, refName, refType, TAG_INFO_ARGS(fileInfo), displayName);
						isprotected = string_ends_with(displayName, NXL_SUFFIX, FALSE);
					}
					else
					{
						DBGLOG(TAG_INFO_FMT":NR[%d/%d]-{refTag=%u refName='%s' refType=%d}-"TAG_INFO_FMT
							, TAG_INFO_ARGS(dsInfo), iref+1, nrefs, refTag, refName, refType, TAG_INFO_ARGS(fileInfo));
					}
					if(displayName != NULL)
					{
						if(!isprotected)
						{
							//not nxl file
							status.nNonNxl++;
						}
						else
						{
							status.nNxl++;
						}
						if (validateFile)
						{
							if (is_reference_require_auto_deletion(dsType, refName))
							{
								char *lowerName = string_toLower2(displayName);
								int iconfict = string_list_index_of(displayNames, lowerName);
								if (iconfict >= 0) {
									TAG_LIST_add(fileToBeDeleted, fileTags->tags[iconfict]);
									string_list_add(refNameToBeDeleted, fileRefNames->items[iconfict]);
									DBGLOG("FileNameConflict with file-%u(refName='%s')"
										, fileTags->tags[iconfict], fileRefNames->items[iconfict]);
								}
								string_list_add(displayNames, lowerName);
								TAG_LIST_add(fileTags, refTag);
								string_list_add(fileRefNames, refName);
							}
						}
					}
					TC_FREE(displayName);
					tag_info_free(fileInfo);
				}
				else
				{
					DBGLOG(TAG_INFO_FMT":NR[%d/%d]-{refTag=%u refName='%s' refType=%d}:InBlackList=%d", TAG_INFO_ARGS(dsInfo), iref+1, nrefs, refTag, refName, refType, inBlacklist);
				}
				TC_FREE(refName);
			}
		}
		string_list_free(nrBlacklist);
		dataset_delete_refs(dsInfo, fileToBeDeleted, refNameToBeDeleted);
		TC_FREE(dsType);
		TAG_LIST_free(fileTags);
		string_list_free(displayNames);
		string_list_free(fileRefNames);
		TAG_LIST_free(fileToBeDeleted);
		string_list_free(refNameToBeDeleted);
	}
	if(updateTool)
	{
		if(status.nNxl > 0
			&& status.nNonNxl == 0)
		{
			//set nxl tool when all files are nxl protected file
			dataset_set_tool(dsInfo, TRUE);
			//set protection status to true when all files are protected
			if(pref_update_protected_on_upload())
				dataset_set_is_protected(dsInfo, true);
		}
		else if(status.nNxl == 0
			&& status.nNonNxl > 0)
		{
			//all files are non-protected, set to default tool if current is nxl tool
			dataset_set_tool(dsInfo, FALSE);
			//dataset_set_is_protected(dsInfo, FALSE);
		}
	}
	DBGLOG(TAG_INFO_FMT":Summary-Total=%d NXL=%d NonNxl=%d"
		, TAG_INFO_ARGS(dsInfo), status.nTotal, status.nNxl, status.nNonNxl);
	return status;
}


//Background:on 2-tier RAC client, file reading will have file downloaded from volume to transient folder instead of using fcc cache
//when the file is very big(e.g. 200mb), the download will cost a lot of time
//Introduce a black list to disable file name validation on dataset saving/importing
BOOL dataset_need_protection_validation(const char *dsType)
{
	//default is ENABLED
	BOOL needValidate = TRUE;
	if(dsType != NULL)
	{
		int ntypes = 0;
		char **types = NULL;
		//get black list from teamcenter preference
		if(ITK_EVAL(PREF_ask_char_values(PREF_VALIDATION_DISABLED_DATASETS, &ntypes, &types)))
		{
			int i;
			NEED_FREE(types);
			DBGLOG(PREF_VALIDATION_DISABLED_DATASETS" has %d values", ntypes);
			for(i = 0; i < ntypes; i++)
			{
				DBGLOG("==>[%d/%d]=%s", i+1, ntypes, types[i]);
				if(types[i] != NULL
					&& _stricmp(types[i], dsType) == 0)//case insensitive
				{
					//in black list
					needValidate = FALSE;
					break;
				}
			}
			TC_FREE(types);
		}
	}
	return needValidate;
}

int Nxl3_dataset_save_post(METHOD_message_t *msg, va_list args)
{
	if (!is_message_suppressed(msg))
	{
		tag_t datasetTag = NULLTAG;
		logical isNewDataset = false;
		tag_info_p dsInfo = NULL;
		logical isCheckout = false;
		char *dsType = NULL;
		TC_HANDLER_START;
		DEBUG_MSG(msg);
		/********************/
		/* Initialization */
		/********************/
		//Get the parameters from the AE_save_dataset
		//Parameters:
		//	 tag_t  datasetTag (I)  
		//	 logical  isNew (I)
		datasetTag = va_arg(args, tag_t);
#if defined(WNT)
		isNewDataset = va_arg(args, logical);
#elif defined(__linux__)
		isNewDataset = va_arg(args, int);//***GCC compile warning: ‘logical {aka unsigned char}’ is promoted to ‘int’ when passed through ‘...’
#endif
		dsInfo = tag_info_new(datasetTag);
		ITK_EVAL(RES_is_checked_out(dsInfo->tagId, &isCheckout));
		ITK_EVAL(WSOM_ask_object_type2(dsInfo->tagId, &dsType));
		NEED_FREE(dsType);
		DBGLOG("AE_save_dataset Args:Dataset="TAG_INFO_FMT" Type='%s' isNew='%d' isCheckout=%d", TAG_INFO_ARGS(dsInfo), dsType, isNewDataset, isCheckout);

		dataset_verify_ref_status(dsInfo, dataset_need_protection_validation(dsType), TRUE);

		tag_info_free(dsInfo);
		TC_FREE(dsType);
		TC_HANDLER_END;
	}
	return ITK_ok;
}