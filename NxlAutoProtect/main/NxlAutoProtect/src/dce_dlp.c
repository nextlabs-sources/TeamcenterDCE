#include <stdarg.h> //va_list() va_start() va_end()
#include <ug_va_copy.h>
#include <tccore/custom.h> //CUSTOM_register_exit()
#include <tc/preferences.h>
#include <ae/dataset.h>
#include <utils_string.h>
#include "dce_log.h" //DCE log
#include "dce_evaluator.h"
#include "dce_mem.h"
#include "dce_dlp.h"


BOOL imanfile_is_whitelisted(tag_info_ro fileInfo)
{
	static hashtable_ro whitelist = NULL;
	BOOL ret = TRUE;
	if(fileInfo == NULL)
		return FALSE;
	if(whitelist == NULL)
	{
		//initializing white list
		char **extensions = NULL;
		int nExts = -1;
		hashtable_p ht = ht_create(10);
		if(ITK_EVAL(PREF_ask_char_values(PREF_DLP_FILES_WHITE_LIST, &nExts, &extensions)))
		{
			int iext;
			NEED_FREE(extensions);
			DBGLOG(PREF_DLP_FILES_WHITE_LIST" constains %d values", nExts);
			for(iext=0; iext<nExts; iext++)
			{
				const char *tolower = CharLower(extensions[iext]);
				DBGLOG("==>%s", tolower);
				ht_set_chars(ht, tolower, "TRUE");
			}
			TC_FREE(extensions)
		}
		whitelist = ht;
	}
	if(whitelist != NULL
		&& whitelist->count > 0)
	{
		//check volume name
		const char *ext = CharLower(PathFindExtension(fileInfo->name));
		DTLLOG("Extension of '%s' is '%s'", fileInfo->name, ext);
		//in white list then return FALSE
		ret = ht_contains_key(whitelist, ext);
	}
	//if whitelist is empty, all files are in whitelisted
	return ret;
}
BOOL is_dataset_whitelisted(tag_t dsTag)
{
	static hashtable_ro whitelist = NULL;
	char *dsType = NULL;
	BOOL ret = TRUE;
	if(dsTag == NULLTAG)
		return FALSE;
	if(whitelist ==NULL)
	{
		//initializing white list
		char **dsTypes = NULL;
		int nTypes = -1;
		hashtable_p ht = ht_create(10);
		if(ITK_EVAL(PREF_ask_char_values(PREF_DLP_DATASETS_WHITE_LIST, &nTypes, &dsTypes)))
		{
			int itype;
			NEED_FREE(dsTypes);
			DBGLOG(PREF_DLP_DATASETS_WHITE_LIST" constains %d values", nTypes);
			for(itype=0; itype<nTypes; itype++)
			{
				const char *tolower = CharLower(dsTypes[itype]);
				DBGLOG("==>%s", tolower);
				ht_set_chars(ht, tolower, "TRUE");
			}
			TC_FREE(dsTypes);
		}
		whitelist = ht;
	}
	if(whitelist != NULL
		&& whitelist->count > 0
		&& ITK_EVAL(WSOM_ask_object_type2(dsTag, &dsType)))
	{
		NEED_FREE(dsType);
		ret = ht_contains_key(whitelist, CharLower(dsType));
		TC_FREE(dsType);
	}

	//if whitelist is empty, all datasets are in whitelisted
	return ret;
}

BOOL is_evaluation_enabled()
{
	static BOOL enabled = NOTSET;
	if(enabled == NOTSET)
	{
		enabled = !pref_get_logical(PREF_DLP_DISABLE_EVALUATION, FALSE);
	}
	return enabled;
}
/*
BOOL is_dlp_enabled()
{
	static BOOL enabled = NOTSET;
	if(enabled == NOTSET)
	{
		solution_info solution;
		enabled = FALSE;
		if((solution = get_solution_info())!=NULL
			&& solution->type==solution_inMotion)
		{
			enabled = TRUE;
		}
	}
	return enabled;
}
*/

int nxl_add_classification_code( int *decision, va_list args )
{
	va_list largs;
	tag_t file_tag = NULLTAG;
	char *file_classification_code = 0;
	tag_info_p fileInfo;

	TC_HANDLER_START;

	va_copy( largs, args );
	file_tag = va_arg( largs, tag_t );
	file_classification_code = va_arg( largs, char* );
	va_end( largs );

	fileInfo = tag_info_new(file_tag);
	*decision = ALL_CUSTOMIZATIONS;
	*file_classification_code = FILE_NON_CLASS;
	DBGLOG("Downloading "TAG_INFO_FMT, TAG_INFO_ARGS(fileInfo));
	//this exit will only be called in dlp solution
	//if(is_dlp_enabled())
	{
		//check file extension white list
		if(imanfile_is_whitelisted(fileInfo))
		{
			//find the owner dataset of this file
			tag_t ownerDatasetTag = imanfile_search_dataset(fileInfo);
			//check dataset type white list
			if(ownerDatasetTag != NULLTAG
				&& is_dataset_whitelisted(ownerDatasetTag))
			{
				//check if evaluation is enabled
				if(is_evaluation_enabled())
				{
					tag_info_p dsInfo = tag_info_new(ownerDatasetTag);
					DBGLOG("Evaluating for Classification code:"TAG_INFO_FMT"("TAG_INFO_FMT")", TAG_INFO_ARGS(dsInfo), TAG_INFO_ARGS(fileInfo));
					//nxl dataset should not exist in dlp solution
					//if(dataset_is_nxl(dsInfo))
					//{
					//	//TODO:TBD:if this dataset is already protected by Nextlabs and the volume file doesn't have .nxl ext
					//	//set classCode to 'X' to ask fcc to append .nxl?
					//	*file_classification_code = 'X';
					//}
					//else
					{
						tag_info_ro revInfo = tag_info_get_parent(dsInfo);
						//evaluate for classification code
						if(evaluate_download(dsInfo, revInfo) == DO_TRANSLATION)
						{
							*file_classification_code = FILE_NEED_PROTECT;
						}
					}
					tag_info_free(dsInfo);
				}
				else
				{
					DBGLOG("Evaluation is skipped!");
					*file_classification_code = FILE_NEED_PROTECT;
				}
				DBGLOG("File: %s Classification code is: '%c'(%d)", fileInfo->name, *file_classification_code, *file_classification_code );
			}
		}
	}
	tag_info_free(fileInfo);
	TC_HANDLER_END;
	return ITK_ok;
}
