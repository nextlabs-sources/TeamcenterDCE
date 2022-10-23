#include <ae/dataset.h>

#include <dce_log.h>
#include <dce_mem.h>
#include <tc_utils.h>
#include <nxl_action_handler.h>

#include <utils_string.h>
#include <utils_system.h>
#include <utils_nxl.h>
#include <NxlSCFIntegration.h>

#ifdef WNT
#define CMD_BCZMODIFIER "bczmodifier.bat"
#else
#define CMD_BCZMODIFIER "bczmodifier.sh"
#endif
BOOL run_modifier(const char *bczfile) {
	char cmd[1024 + 1];
	sprintf_s(cmd, 1024, "\"%s\" \"%s\"", search_dce_lib(CMD_BCZMODIFIER), bczfile);
	if (process_read_output(cmd, NULL, 2000))
	{
		return TRUE;
	}
	return FALSE;
}

int Nxl3_bcz_modifier(METHOD_message_t *msg, va_list args)
{
	if (!is_message_suppressed(msg))
	{
		tag_t datasetTag = NULLTAG;
		logical isNewDataset = false;
		tag_info_p dsInfo = NULL;
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
		ITK_EVAL(WSOM_ask_object_type2(dsInfo->tagId, &dsType));
		NEED_FREE(dsType);
		DBGLOG("AE_save_dataset Args:Dataset=" TAG_INFO_FMT " Type='%s' isNew='%d'", TAG_INFO_ARGS(dsInfo), dsType, isNewDataset);
		
		if (isNewDataset
			//&& stricmp("Briefcase", dsType)
			)
		{
			//find ref
			tag_info_p fileInfo = NULL;
			if (dataset_find_nr_info(dsInfo, "Briefcase", 0, NULL, &fileInfo))
			{
				//1, download bcz file to temp
				//2, run bczmodifier
				//3, upload new bcz file
				//4, delete temp file
				char tempFile[SS_MAXPATHLEN + 1];
				char *displayName = NULL;
				if (GetTempPath(SS_MAXPATHLEN, tempFile) <= 0) return false;
				ITK_EVAL(IMF_ask_original_file_name2(fileInfo->tagId, &displayName));
				strcat_s(tempFile, SS_MAXPATHLEN, displayName);
				if (ITK_EVAL(IMF_export_file(fileInfo->tagId, tempFile)))
				{
					DBGLOG("Downloaded:'%s'", tempFile);
					if (run_modifier(tempFile)) {
						if (ITK_EVAL(IMF_replace_file(fileInfo->tagId, tempFile, true)))
						{
							DBGLOG("Replaced:'%s'", tempFile);
						}
					}
					//TODO
					//DeleteFile(tempFile);
				}
				TC_FREE(displayName);
			}

		}

		tag_info_free(dsInfo);
		TC_FREE(dsType);
		TC_HANDLER_END;
	}
	return ITK_ok;
}