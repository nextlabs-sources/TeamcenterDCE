#include <stdarg.h> //va_list() va_start() va_end()
#include "dce_mem.h"
#include <tccore/custom.h>
#include <ae/dataset.h>
#include <ae/datasettype.h>
#include <tccore/item.h>
#include <tccore/grm.h>

#include <dce_log.h>
#include <utils_string.h>
#include <tc_utils.h>
#include <utils_system.h>

#include <nxrmx_pdm_server.h>
#include <NxlSCFIntegration.h>
#include <string>
#include <vector>
#include <NxlUtils/string_utils.hpp>

#include <NxlUtils/TonxlfileTranslator.hpp>


char msgBuffer[1024];
#define SUCCESS_PROCESSED(n) sprintf_s(msgBuffer, 1024, "Successfully protected %d datasets", n)
#define ERROR_INVALID_FORMAT "Error-invalid format"
#define ERROR_DATASET_NOT_FOUND "Error-dataset not found"
#define ERROR_ITEM_REV_NOT_FOUND "Error-item revision not found"
tag_info_p find_item_revision(const char *itemid, const char *itemrev)
{
	tag_t found;
	DBGLOG("Searching ItemRevision with itemid='%s' itemrev='%s'", itemid, itemrev);
	if (ITK_EVAL(ITEM_find_rev(itemid, itemrev, &found)))
	{
		if (found != NULL_TAG)
		{
			return tag_info_new(found);
		}
	}
	return NULL;
}
tag_info_p find_ugmaster(const char *itemid, const char *itemrev, const char *dsType="UGMASTER") {

	DBGLOG("Searching %s dataset with itemid='%s' itemrev='%s'", dsType, itemid, itemrev);
	tag_t found;
	if (ITK_EVAL(ITEM_find_rev(itemid, itemrev, &found)))
	{
		tag_info_p revInfo = tag_info_new(found);
		int cntSecObj;
		GRM_relation_t *secObjs;
		//get children objects from ItemRevision
		if (ITK_EVAL(GRM_list_secondary_objects(revInfo->tagId, NULLTAG, &cntSecObj, &secObjs))
			&& cntSecObj > 0)
		{
			int iSecObj;
			for (iSecObj = 0; iSecObj < cntSecObj; iSecObj++)
			{
				tag_info_p dsInfo = tag_info_new(secObjs[iSecObj].secondary);
				DBGLOG("[%d/%d]" TAG_INFO_FMT, iSecObj + 1, cntSecObj, TAG_INFO_ARGS(dsInfo));
				if (dsInfo->rootType == type_dataset)
				{
					char *typeName = NULL;
					if (ITK_EVAL(WSOM_ask_object_type2(dsInfo->tagId, &typeName)))
					{
						DBGLOG("==>typeName='%s'", typeName);
						if (_stricmp(typeName, dsType) == 0) {
							TC_FREE(typeName);
							return dsInfo;
						}
					}
					TC_FREE(typeName);
				}
				tag_info_free(dsInfo);
			}
		}
	}
	return NULL;
}

bool imanfile_protect(tag_info_ro fileInfo, const std::vector<std::pair<std::string, std::string>> &tags) {
	BOOL isprotected = FALSE;
	if (imanfile_is_protected_fsc(fileInfo, &isprotected))
	{
		if (isprotected) {
			DBGLOG(TAG_INFO_FMT "file is protected already.", TAG_INFO_ARGS(fileInfo));
			return true;
		}
	}
	else
	{
		return false;
	}
	char *displayName = NULL;
	if (!ITK_EVAL(IMF_ask_original_file_name2(fileInfo->tagId, &displayName)))
		return false;
	char temp[SS_MAXPATHLEN + 1];
	if (GetTempPath(SS_MAXPATHLEN, temp) <= 0)
		return false;
	const std::string &workingFolder = BuildPath(std::string(temp), std::string(PROJECT_NAME));
	mkpath(workingFolder);
	//download file to temp folder
	std::string downloadFile = BuildPath(workingFolder, std::string(displayName));
	std::remove(downloadFile.c_str());
	if (ITK_EVAL(IMF_export_file(fileInfo->tagId, downloadFile.c_str())))
	{
		DBGLOG("Downloaded:'%s'", downloadFile.c_str());
		//protect file
		std::string protectedFile = workingFolder;
		if (TonxlfileTranslator::protect(downloadFile, tags, protectedFile)) {
			DBGLOG("Protected:'%s'", protectedFile.c_str());
			if (ITK_EVAL(IMF_replace_file(fileInfo->tagId, protectedFile.c_str(), false)))
			{
				DBGLOG("Replaced:'%s'", protectedFile.c_str());
			}
			std::remove(protectedFile.c_str());
		}
	}
	std::remove(downloadFile.c_str());
	TC_FREE(displayName);
	return true;
}

std::vector<std::pair<std::string, std::string>> dataset_get_nxl_attributes(tag_info_ro dsInfo) {
	kvl_p tags = dataset_get_protect_attributes(dsInfo);
	std::vector<std::pair<std::string, std::string>> vtags;
	if (tags != NULL) {
		for (int i = 0; i < tags->count; i++) {
			vtags.push_back(std::make_pair(tags->keys[i], tags->vals[i]));
		}
	}
	kvl_free(tags);
	return vtags;
}

int dataset_protect(tag_info_ro dsInfo)
{
	int nfiles = 0;
	if (dsInfo != NULL
		&&dsInfo->rootType == type_dataset
		//TODO:TBD
		//&& dataset_is_nxl(dsInfo)
		)
	{
		int nrefs = 0;
		if (ITK_EVAL(AE_ask_dataset_ref_count(dsInfo->tagId, &nrefs))
			&& nrefs > 0)
		{
			int iref;
			string_list_p nrBlacklist = pref_nr_blacklist();
			char *dsType = dataset_get_type(dsInfo);
			auto nxltags = dataset_get_nxl_attributes(dsInfo);
			for (iref = 0; iref < nrefs; iref++)
			{
				char *refName;
				AE_reference_type_t refType;
				tag_t refTag;
				if (ITK_EVAL(AE_find_dataset_named_ref2(dsInfo->tagId, iref, &refName, &refType, &refTag)))
				{
					BOOL inBlacklist = string_list_index_of(nrBlacklist, refName) >= 0;
					NEED_FREE(refName);
					if (!inBlacklist)
					{
						tag_info_p fileInfo = tag_info_new(refTag);
						DBGLOG(TAG_INFO_FMT ":NR[%d/%d]-{refTag=%u refName='%s' refType=%d}-" TAG_INFO_FMT
							, TAG_INFO_ARGS(dsInfo), iref + 1, nrefs, refTag, refName, refType, TAG_INFO_ARGS(fileInfo));
						if (imanfile_protect(fileInfo, nxltags))
						{
							nfiles++;
						}
						tag_info_free(fileInfo);
					}
					else
					{
						DBGLOG(TAG_INFO_FMT":NR[%d/%d]-{refTag=%u refName='%s' refType=%d}:InBlackList=%d", TAG_INFO_ARGS(dsInfo), iref + 1, nrefs, refTag, refName, refType, inBlacklist);
					}
					TC_FREE(refName);
				}
			}
			if (nfiles > 0) {
				//udpate tool used
				dataset_set_tool(dsInfo, TRUE);
				//TODO:mark dataset as nextlabs protected
			}
			string_list_free(nrBlacklist);
			TC_FREE(dsType);
		}
	}
	return nfiles;
}

extern int NXRMX_invoke_pdm_server(int *decision, va_list args)
{
	TC_HANDLER_START;
	/***********  va_list for USER_invoke_pdm_server ***********/
	int  input_code = va_arg(args, int);          /* args 1 */
	char *input_string = va_arg(args, char *);    /* args 2 */
	int  *output_code = va_arg(args, int *);      /* args 3 */
	char **output_string = va_arg(args, char **); /* args 4 */
												  /***********************************************************/

	DBGLOG("incoming=============================================");
	DBGLOG(" *decision = %d", *decision);
	DBGLOG(" input_code = %#X", input_code);
	DBGLOG(" input_string = %s", input_string);

	*output_code = OUTPUT_CODE_ERROR_BASE;
	std::string ostring;
	switch (input_code)
	{
	case INPUT_CODE_PROTECT:
		*decision = ONLY_CURRENT_CUSTOMIZATION;
		if (string_starts_with(input_string, "@DB/"))
		{
			string_list_p list = string_list_new();
			int n = string_split(input_string, "/", list);
			if (n >= 3)
			{
				tag_info_p ugmasterInfo = find_ugmaster(list->items[1], list->items[2]);
				if (ugmasterInfo != NULL)
				{
					DBGLOG("Found-ItemRevision:" TAG_INFO_FMT, TAG_INFO_ARGS(ugmasterInfo));

					int nprotected = dataset_protect(ugmasterInfo);

					tag_info_free(ugmasterInfo);

					*output_code = OUTPUT_CODE_SUCCESS;
					SUCCESS_PROCESSED(nprotected);
					ostring = msgBuffer;
				}
				else
				{
					ostring = ERROR_ITEM_REV_NOT_FOUND;
				}
			}
			else
			{
				ostring = ERROR_INVALID_FORMAT;
			}
			string_list_free(list);
		}
		else
		{
			ostring = ERROR_INVALID_FORMAT;
		}
		break;
	default:
		*decision = ALL_CUSTOMIZATIONS;
		break;
	}

	// allocate memory for output string and copy 
	// std::string's contents to it:
	*output_string = MEM_string_copy (ostring.c_str());
	DBGLOG("*decision = %d", *decision);
	DBGLOG("output_code = %#X", *output_code);
	DBGLOG("*output_string = '%s'", *output_string);
	DBGLOG("outgoing-------------------------------------------------");

	TC_HANDLER_END;
	return ITK_ok;
}