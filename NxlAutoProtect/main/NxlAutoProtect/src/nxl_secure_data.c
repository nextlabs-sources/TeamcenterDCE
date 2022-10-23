
#include <tc_utils.h>

#include <dce_evaluator.h>
#include <dce_translator.h>


#include <tc/preferences.h>

#include <dce_mem.h>
#include <dce_log.h>
#include <utils_string.h>

#define DRM_EVAL_SECUREDATA "RIGHT_EXECUTE_SECURE_DATA"

/*
Secure Data regionally related
Dataset Types is not cached that doesn't require restart RAC
*/
#define PREF_SECURE_DATA_DATASETS "NXL_Secure_Data_Dataset_Types"

#define PREF_SECURE_DATA_DEFAULT_ACTION "NXL_Secure_Data_Default_Action"
#define PREF_SECURE_DATA_DEFAULT_PROTECT "protect"
#define PREF_SECURE_DATA_DEFAULT_SKIP "skip"

int evaluate_secure_data(tag_info_ro dsInfo) {

	obligation_list obligations = NULL;
	int nob;

	if (evaluate_internal(DRM_EVAL_SECUREDATA, dsInfo, NULL, NULL, &obligations, &nob) != EVAL_DEFAULT)
	{
		int iob;
		int validObs = 0;
		for (iob = 0; iob < nob; iob++)
		{
			DBGLOG("Obligation[%d/%d]%s:%s", iob + 1, nob, obligations[iob].name, obligations[iob].policy);
			if (strcmp(obligations[iob].name, DRM_OB_NAME) == 0)
			{
				validObs++;
			}
		}
		//free obligations
		obligations_free(obligations, nob);
		if (validObs > 0)
			return DO_TRANSLATION;
		return SKIP_TRANSLATION;
	}
	return DO_DEFAULT;
}
logical is_secure_data_enabled(tag_info_ro tagInfo) {
	logical ret = false;
	if (tagInfo != NULL && tagInfo->rootType == type_dataset)
	{
		char *datasetType = dataset_get_type(tagInfo);
		DBGLOG(TAG_INFO_FMT ":DatasetType='%s'", TAG_INFO_ARGS(tagInfo), datasetType);
		if (datasetType != NULL)
		{
			int ntypes = 0;
			char **types = NULL;
			if (ITK_EVAL(PREF_ask_char_values(PREF_SECURE_DATA_DATASETS, &ntypes, &types)))
			{
				int i;
				NEED_FREE(types);
				DBGLOG(PREF_SECURE_DATA_DATASETS" has %d values", ntypes);
				for (i = 0; i < ntypes; i++)
				{
					DBGLOG("==>[%d/%d]=%s", i + 1, ntypes, types[i]);
					if (string_wildcmp(datasetType, types[i]))
					{
						ret = true;
					}
				}
				TC_FREE(types);
			}
			TC_FREE(datasetType);
		}
	}
	return ret;
}

logical dataset_need_secure(tag_info_ro tagInfo, logical *oIsDefault)
{
	logical ret = false;
	if (oIsDefault != NULL)
	{
		*oIsDefault = FALSE;
	}
	if (is_secure_data_enabled(tagInfo))
	{
		//step 2: evaluate
		switch (evaluate_secure_data(tagInfo)) {
		case DO_TRANSLATION:
			ret = TRUE;
			break;
		case SKIP_TRANSLATION:
			ret = FALSE;
			break;
		default:
			if (oIsDefault != NULL)
			{
				*oIsDefault = TRUE;
			}
			{
				const char *defaultAction = pref_get_char(PREF_SECURE_DATA_DEFAULT_ACTION, PREF_SECURE_DATA_DEFAULT_SKIP);
				if (defaultAction != NULL
					&& _stricmp(defaultAction, PREF_SECURE_DATA_DEFAULT_PROTECT) == 0) {
					ret = TRUE;
				}
			}
			break;
		}
	}
	return ret;
}


tag_t secure_dataset(tag_info_ro tagInfo, char context[200]) {
	tag_t translation = NULLTAG;
	logical isDefault = false;
	if (dataset_need_secure(tagInfo, &isDefault)) {

		request_type syncOp = request_protect;
		request_priority priority = priority_high;
		char reason[300];
		if (isDefault)
		{
			DBGLOG(TAG_INFO_FMT" need to be secured!(default)", TAG_INFO_ARGS(tagInfo));
			sprintf_s(reason, sizeof(reason), "secure data on %s(default)",context);
		}
		else
		{
			DBGLOG(TAG_INFO_FMT" need to be secured!", TAG_INFO_ARGS(tagInfo));
			sprintf_s(reason, sizeof(reason), "secure data on %s", context);
		}
		//sent translation request
		CALL_DBG(translation = send_translation_request(tagInfo, NULL, NULL, 0, syncOp, priority, reason));
	}
	return translation;
}