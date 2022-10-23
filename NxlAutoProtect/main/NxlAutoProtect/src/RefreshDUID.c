#include <dce_mem.h>
#include <dce_log.h>
#include <nxl_action_handler.h>

#include <tccore/item_msg.h>
#include <tccore/wso_msg.h>
#include <tccore/grm.h>
#include <ae/dataset.h>

#include <tc_utils.h>
#include <dce_translator.h>
#include <utils_string.h>

typedef struct list_dataset_callback_data_s{
	tag_list_p datasets;
}list_dataset_callback_data_t, * list_dataset_callback_data_p;
BOOL list_dataset_callback(EnumerateCallbackParams_p params) {
	tag_info_p dsInfo = tag_info_new(params->itemTag);
	list_dataset_callback_data_p data = (list_dataset_callback_data_p)params->cbData;
	if (dsInfo != NULL && data != NULL)
	{
		DBGLOG(TAG_INFO_FMT, TAG_INFO_ARGS(dsInfo));
		if (dsInfo->rootType == type_dataset)
		{
			TAG_LIST_add(data->datasets, params->itemTag);
		}
		CALL_DTL(tag_info_free(dsInfo));
	}
	return false;
}
typedef struct find_nxl_reference_callback_data_s {
	BOOL foundProtectedFile;
}find_nxl_reference_callback_data_t, * find_nxl_reference_callback_data_p;
BOOL find_nxl_reference_callback(EnumNRCallbackParams_p params) {
	if (params->isblacklisted)
		return false;
	tag_info_p fileInfo = tag_info_new(params->refTag);
	if (fileInfo != NULL) {
		BOOL isprotected = false;
		DBGLOG(TAG_INFO_FMT, TAG_INFO_ARGS(fileInfo));
		if (imanfile_is_protected_fsc(fileInfo, &isprotected)
			&& isprotected) {
			find_nxl_reference_callback_data_p data = (find_nxl_reference_callback_data_p)params->cbData;
			if (data != NULL) {
				data->foundProtectedFile = TRUE;
			}
			return true;
		}
	}
	return false;
}
BOOL refresh_duid(tag_info_ro dsTag) {
	if (dsTag == NULL)
		return FALSE;
	if (dsTag->rootType != type_dataset)
		return false;
	//if dataset has no protected file, return false
	//send update duid request 
	return true;
}
int on_item_copy_rev(METHOD_message_t* msg, va_list args) {
	int ifail = ITK_ok;

	TC_HANDLER_START;
	DEBUG_MSG(msg);
	//Parameters:
	//	tag_t 	source_rev
	//		const 	char* rev_id
	//		tag_t* new_rev
	//		tag_t 	item_rev_master_form
	tag_t sourceRevTag = va_arg(args, tag_t);
	const char* revId = va_arg(args, const char*);
	tag_t *pNewRevTag = va_arg(args, tag_t*);
	tag_t masterformTag =va_arg(args, tag_t);

	DBGLOG("sourceRevTag='%u'", sourceRevTag);
	DBGLOG("rev_id='%s'", revId);
	if (pNewRevTag != NULL) {
		DBGLOG("*pNewRevTag='%u'", *pNewRevTag);
	}
	else
	{
		DBGLOG("pNewRevTag=NULL");
	}
	DBGLOG("masterformTag='%u'", masterformTag);
	if (sourceRevTag != NULLTAG) {
	}
	if (pNewRevTag != NULL
		&& *pNewRevTag != NULLTAG) {
		tag_info_p newRev = tag_info_new(*pNewRevTag);
		tag_info_p srcRev = tag_info_new(sourceRevTag);
		list_dataset_callback_data_t listSrcRevDatasetData = { TAG_LIST_new() };
		list_dataset_callback_data_t listNewRevDatasetData = { TAG_LIST_new() };
		if (srcRev != NULL)
		{
			DBGLOG("Enumerating SourceRev:" TAG_INFO_FMT, TAG_INFO_ARGS(srcRev));
		}
		EnumerateItemRevDatasets(sourceRevTag, list_dataset_callback, &listSrcRevDatasetData);
		if (newRev != NULL) {
			DBGLOG("Enumerating NewRev:" TAG_INFO_FMT, TAG_INFO_ARGS(newRev));
		}
		EnumerateItemRevDatasets(*pNewRevTag, list_dataset_callback, &listNewRevDatasetData);
		//
		if (listNewRevDatasetData.datasets != NULL
			&& listNewRevDatasetData.datasets->count > 0) {
			int idataset;
			find_nxl_reference_callback_data_t cbData = { FALSE };
			for (idataset = 0; idataset < listNewRevDatasetData.datasets->count; idataset++) {
				tag_t dsTag = listNewRevDatasetData.datasets->tags[idataset];
				//check if this dataset exist in source dataset, 
				if (!TAG_LIST_contains(listSrcRevDatasetData.datasets, dsTag)) {
					tag_info_p dsInfo = tag_info_new(dsTag);
					cbData.foundProtectedFile = FALSE;
					DBGLOG("Enumerating Dataset:" TAG_INFO_FMT, TAG_INFO_ARGS(dsInfo));
					//if no, which means this dataset is revisedand need to be refresh duid
					EnumerateDatasetReferences(dsTag, find_nxl_reference_callback, &cbData);
					if (cbData.foundProtectedFile) {
						//send refresh duid translation request
						send_translation_request(dsInfo, newRev, NULL, 0, request_refreshduid, priority_high, "on copy rev");
					}
					CALL_DTL(tag_info_free(dsInfo));
				}
			}
		}
		TAG_LIST_free(listSrcRevDatasetData.datasets);
		TAG_LIST_free(listNewRevDatasetData.datasets);
		tag_info_free(newRev);
		tag_info_free(srcRev);
	}
	TC_HANDLER_END;
	return ifail;
}
int post_wso_copy(METHOD_message_t* msg, va_list args)
{
	int ifail = ITK_ok;

	TC_HANDLER_START;

	DEBUG_MSG(msg);
	/*@param tag_t       wso
		@param const char* new_name
		@param tag_t* new_wso*/
	tag_t wsoTag = va_arg(args, tag_t);
	const char* newName = va_arg(args, const char*);
	tag_t* pNewWsoTag = va_arg(args, tag_t*);

	DBGLOG("wsoTag='%u'", wsoTag);
	tag_info_p srcWsoInfo = tag_info_new(wsoTag);
	if (srcWsoInfo != NULL)
	{
		DBGLOG("SourceWSO:" TAG_INFO_FMT, TAG_INFO_ARGS(srcWsoInfo));
		DBGLOG("pNewWsoTag='%p'", pNewWsoTag);
		//DBGLOG("newName='%s'", newName);
		if (pNewWsoTag != NULL) {
			DBGLOG("*pNewWsoTag='%u'", *pNewWsoTag);
			if (*pNewWsoTag != NULLTAG) {
				tag_info_p newWsoInfo = tag_info_new(*pNewWsoTag);
				if (newWsoInfo != NULL) {
					DBGLOG("NewWSO:" TAG_INFO_FMT, TAG_INFO_ARGS(newWsoInfo));
					if (srcWsoInfo->rootType == type_dataset
						&& newWsoInfo->rootType == type_dataset) {
						find_nxl_reference_callback_data_t cbData = { FALSE };
						DBGLOG("Enumerating Dataset:" TAG_INFO_FMT, TAG_INFO_ARGS(newWsoInfo));
						//if no, which means this dataset is revisedand need to be refresh duid
						EnumerateDatasetReferences(newWsoInfo->tagId, find_nxl_reference_callback, &cbData);
						if (cbData.foundProtectedFile) {
							//send refresh duid translation request
							send_translation_request(newWsoInfo, NULL, NULL, 0, request_refreshduid, priority_high, "post wso copy");
						}
					}
					tag_info_free(newWsoInfo);
				}
			}
		}
		else
		{
			DBGLOG("pNewWsoTag=NULL");
		}
		tag_info_free(srcWsoInfo);
	}


	TC_HANDLER_END;
	return ifail;
}


void refresh_duid_install() {
	//REGISTER_EVENT_HANDLER("ItemRevision", NULL, ITEM_copy_rev_msg, METHOD_post_action_type, on_item_copy_rev);
	//REGISTER_EVENT_HANDLER("ItemRevision", NULL, ITEM_copy_rev_msg, METHOD_pre_action_type, on_item_copy_rev);
	//REGISTER_EVENT_HANDLER("ItemRevision", NULL, ITEM_copy_rev_to_existing_msg, METHOD_post_action_type, nxl_debug);
	//REGISTER_EVENT_HANDLER("ItemRevision", NULL, ITEM_create_rev_msg, METHOD_post_action_type, nxl_debug);
	//REGISTER_EVENT_HANDLER("Item", NULL, ITEM_create_msg, METHOD_post_action_type, nxl_debug);
	//REGISTER_EVENT_HANDLER("Item", NULL, ITEM_create_from_rev_msg, METHOD_post_action_type, nxl_debug);
	REGISTER_EVENT_HANDLER("Dataset", NULL, WSO_copy_msg, METHOD_post_action_type, post_wso_copy);
}