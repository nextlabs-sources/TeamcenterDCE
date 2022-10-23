#include <dce_mem.h>
#include <dce_log.h>
#include <nxl_action_handler.h>
#include <nxl_secure_data.h>

#include <ae/dataset.h>
#include <ae/dataset_msg.h>
#include <res/res_itk.h>

#include <tc_utils.h>
#include <utils_string.h>

int Nxl3_metadata_sync_on_import_post(METHOD_message_t *msg, va_list args)
{
	tag_t datasetTag = NULLTAG;
	char *dsType = NULL;
	char *refName = NULL;
	AE_reference_type_t refType;
	char * osFullPath = NULL;
	char *fileName = NULL;
	int fileType = 0;
	logical isCheckout = false;
	tag_info_p dsInfo = NULL;

	TC_HANDLER_START;
	DEBUG_MSG(msg);
	/********************/
	/* Initialization */
	/********************/
	//Get the parameters from the AE_import_file
	//Parameters:
	//	 tag_t  datasetTag (I)  
	//	 logical  isNew (I)
	datasetTag = va_arg(args, tag_t);
	refName = va_arg(args, char *);
	refType = va_arg(args, AE_reference_type_t);
	osFullPath = va_arg(args, char *);
	fileName = va_arg(args, char *);
	fileType = va_arg(args, int);
	dsInfo = tag_info_new(datasetTag);
	dsType = dataset_get_type(dsInfo);
	ITK_EVAL(RES_is_checked_out(dsInfo->tagId, &isCheckout));
	DBGLOG("AE_import_file Args:Dataset="TAG_INFO_FMT" refName='%s' refType='%d' osFullPath='%s', fileName='%s' fileType='%d'(isCheckout=%d)"
		, TAG_INFO_ARGS(dsInfo)
		, refName
		, refType
		, osFullPath
		, fileName
		, fileType
		, isCheckout);

	if (is_reference_require_metadatasync(dsType, refName)) {
		//find ref
		tag_info_p fileInfo = NULL;
		if (dataset_find_nr_info(dsInfo, refName, refType, path_find_name(osFullPath), &fileInfo))
		{
			kvl_p tags = NULL;
			//TODO:find the named reference object
			if (imanfile_read_metadata_fsc(fileInfo, &tags)
				&& tags != NULL) {
				//check dataset properties
				int ikey;
				for (ikey = 0; ikey < tags->count; ikey++) {
					dataset_update_attr_value(dsInfo, tags->keys[ikey], tags->vals[ikey], FALSE);
				}
			}
		}
		else
		{
			DBGLOG("Failed to find the target named reference");
		}
	}
	if (!isCheckout)
	{
		secure_dataset(dsInfo, "file import");
	}

	tag_info_free(dsInfo);
	TC_FREE(dsType);
	TC_HANDLER_END;
	return ITK_ok;
}

int metadatasync_to_dataset_on_import()
{
	return REGISTER_EVENT_HANDLER("Dataset", NULL, AE_import_file_msg, METHOD_post_action_type, Nxl3_metadata_sync_on_import_post);
}