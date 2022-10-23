#include <dce_mem.h>
#include <dce_log.h>
#include <nxl_action_handler.h>

#include <tccore/item.h>
#include <tccore/aom_prop.h>
#include <tccore/license.h>
#include <tccore/grm.h>
#include <res/res_itk.h>

#include <tc_utils.h>
#include "utils_string.h"
#include <dce_translator.h>

#define PROP_LICENSE_LIST "license_list"
#define MESSAGE_LICENSE_ATTACH "TC_attach_ada_license"
#define MESSAGE_LICENSE_DETACH "TC_detach_ada_license"

static int sync_dataset(tag_info_ro dsInfo, tag_info_ro revInfo, tag_list_p ctx)
{
	tag_t request = NULLTAG;
	if (dsInfo != NULL
		&&dsInfo->rootType == type_dataset)
	{
		request_type syncOp = request_non;
		request_priority priority = priority_low;
		const char *reason = NULL;
		ref_status_t fileStatus = { 0 };
		if (TAG_LIST_contains(ctx, dsInfo->tagId))
		{
			DBGLOG(TAG_INFO_FMT" has been synced in this session!", TAG_INFO_ARGS(dsInfo));
			return 0;
		}
		//check if all the named refs are nxl files
		fileStatus = dataset_verify_ref_status(dsInfo, FALSE, FALSE);
		//updateTag
		if (fileStatus.nNxl > 0
			&& dataset_is_nxl(dsInfo))
		{
			DBGLOG(TAG_INFO_FMT" need to be updated!", TAG_INFO_ARGS(dsInfo));
			reason = "updateTags on license changed";
			priority = priority_medium;
			syncOp = request_update;
		}
		if (syncOp == request_non)
		{
			DBGLOG(TAG_INFO_FMT" DONT need to be synced!", TAG_INFO_ARGS(dsInfo));
			goto PROCESSED;
		}
		if (revInfo == NULL)
		{
			revInfo = tag_info_get_parent(dsInfo);
		}
		//sent translation request
		CALL_DBG(request = send_translation_request(dsInfo, revInfo, NULL, 0, syncOp, priority, reason));

	PROCESSED:
		CALL_DBG(TAG_LIST_add(ctx, dsInfo->tagId));
	}
	return request == NULLTAG ? 0 : 1;
}

static int sync_item_revision(tag_info_ro revInfo, tag_list_p ctx)
{
	int synced = 0;
	if (revInfo != NULL
		&& revInfo->rootType == type_item_revision)
	{
		int cntSecObj, iSecObj;
		GRM_relation_t *secObjs;
		//get children objects from ItemRevision
		if (ITK_EVAL(GRM_list_secondary_objects(revInfo->tagId, NULLTAG, &cntSecObj, &secObjs))
			&& cntSecObj > 0)
		{
			for (iSecObj = 0; iSecObj < cntSecObj; iSecObj++)
			{
				tag_info_p dsInfo = tag_info_new(secObjs[iSecObj].secondary);
				DBGLOG(TAG_INFO_FMT":SecObj[%d/%d]-"TAG_INFO_FMT"relationType='%u'"
					, TAG_INFO_ARGS(revInfo), iSecObj + 1, cntSecObj, TAG_INFO_ARGS(dsInfo), secObjs[iSecObj].relation_type);
				if (dsInfo->rootType == type_dataset)
				{
					//for other dataset, sync them when ItemRevision propagation is enabled AND
					//ItemRevision doesn't have snapshot which mean this is a force update
					//OR ItemRevision have snapshot and has changes on propagation properties
					logical isCheckout = false;
					//skip dataset which is still checking out in force
					ITK_EVAL(RES_is_checked_out(dsInfo->tagId, &isCheckout));
					if (!isCheckout)
					{
						synced += sync_dataset(dsInfo, revInfo, ctx);
					}
					else
					{
						DBGLOG(TAG_INFO_FMT":Dataset is checked out now, will be sync after manual checkedin", TAG_INFO_ARGS(dsInfo));
					}
				}
				CALL_DTL(tag_info_free(dsInfo));
			}//End for(iSecObj = 0; iSecObj < cntSecObj; iSecObj++)

			TC_FREE(secObjs);
		}
		else
		{
			DBGLOG(TAG_INFO_FMT":has 0 Secondary Objects", TAG_INFO_ARGS(revInfo));
		}
	}
	return synced;
}
int nxl_ada_license_handler(METHOD_message_t *msg, va_list args)
{
	int ifail = ITK_ok;
	tag_t licenseTag = NULLTAG;
	const char *eadParagraph = NULL;

	TC_HANDLER_START;

	DEBUG_MSG(msg);

	if ((licenseTag = va_arg(args, tag_t)) != NULLTAG)
	{
		int nobj = 0;
		tag_t *objTags = NULL;
		char *buf = NULL;
		if (ITK_EVAL(ADA_ask_license_id2(licenseTag, &buf)))
		{
			DBGLOG("%u:ID='%s'", licenseTag, buf);
			TC_FREE(buf);
		}
		if (ITK_EVAL(ADA_ask_license_category(licenseTag, &buf)))
		{
			DBGLOG("%u:Category='%s'", licenseTag, buf);
			TC_FREE(buf);
		}
		if (ITK_EVAL(ADA_ask_license_reason2(licenseTag, &buf)))
		{
			DBGLOG("%u:Reason='%s'", licenseTag, buf);
			TC_FREE(buf);
		}
		if (ITK_EVAL(ADA_list_license_objects(licenseTag, &nobj, &objTags))) {
			DBGLOG("%u:nobj=%d", licenseTag, nobj);
		}
	}
	if (_stricmp(GET_MSGNAME(msg), MESSAGE_LICENSE_ATTACH) == 0
		&& (eadParagraph = va_arg(args, const char*)) != NULLTAG)
	{
		DBGLOG("EadParagraph='%s'", eadParagraph);
	}
	if (msg->object_tag != NULLTAG)
	{
		int synced = 0;
		tag_info_p tagInfo = tag_info_new(msg->object_tag);
		tag_list_p context = TAG_LIST_new();
		logical isCheckout = false;
		//TODO:when attach/detach multiple license from object, this will be triggered multiple times for each license
		//try to avoid duplicated translation request
		DBGLOG(TAG_INFO_FMT":ADA License changed", TAG_INFO_ARGS(tagInfo));
		{
			int nvalues = 0;
			char **propValues = NULL;
			if (ITK_EVAL(AOM_ask_displayable_values(tagInfo->tagId, PROP_LICENSE_LIST,&nvalues, &propValues)))
			{
				int i;
				NEED_FREE(propValues);
				DBGLOG(TAG_INFO_FMT":'%s' has %d values",
					TAG_INFO_ARGS(tagInfo), PROP_LICENSE_LIST, nvalues);
				for (i = 0; i < nvalues; i++)
				{
					DBGLOG(TAG_INFO_FMT":'%s'[%d/%d]='%s'",
						TAG_INFO_ARGS(tagInfo), PROP_LICENSE_LIST, i+1, nvalues, propValues[i]);
				}
				TC_FREE(propValues);
			}
		}
		ITK_EVAL(RES_is_checked_out(tagInfo->tagId, &isCheckout));
		DBGLOG(TAG_INFO_FMT":isCheckout=%d", TAG_INFO_ARGS(tagInfo), isCheckout);
		//skip itemrevision is still checking out
		if (!isCheckout)
		{
			switch (tagInfo->rootType) {
			case type_item:
				{
					int cntRevs;
					tag_t *listRevs;
					//get children ItemRevisions from Item
					if (ITK_EVAL(ITEM_find_revisions(tagInfo->tagId, "*", &cntRevs, &listRevs))
						&& cntRevs > 0)
					{
						int iRev;
						NEED_FREE(listRevs);
						for (iRev = 0; iRev < cntRevs; iRev++)
						{
							tag_info_p revInfo = tag_info_new(listRevs[iRev]);
							if (revInfo->rootType == type_item_revision)
							{
								logical isRevCheckout = false;
								//skip itemrevision is still checking out
								ITK_EVAL(RES_is_checked_out(revInfo->tagId, &isRevCheckout));
								DBGLOG(TAG_INFO_FMT":Revision[%d/%d]-"TAG_INFO_FMT" checkedOut=%d", TAG_INFO_ARGS(tagInfo), iRev + 1, cntRevs, TAG_INFO_ARGS(revInfo), isRevCheckout);
								if (!isRevCheckout)
								{
									//proces item revision without checking n
									synced = sync_item_revision(revInfo, context);
								}
							}
							else
							{
								DBGLOG(TAG_INFO_FMT":Revision[%d/%d]-"TAG_INFO_FMT, TAG_INFO_ARGS(tagInfo), iRev + 1, cntRevs, TAG_INFO_ARGS(revInfo));
							}
							CALL_DTL(tag_info_free(revInfo));
						}

						TC_FREE(listRevs);
					}
				}
				break;
			case type_item_revision:
				synced = sync_item_revision(tagInfo, context);
				break;
			case type_dataset:
				synced = sync_dataset(tagInfo, NULL, context);
				break;
			default:
				break;
			}
		}
		CALL_DTL(tag_info_free(tagInfo));
	}

	TC_HANDLER_END;
	return ifail;
}


int metadatasync_on_license_changed()
{
	if (pref_metadtasync_enabled())
	{
		kvl_p metadata = pref_get_protect_attributes();
		if (kvl_indexer(metadata, PROP_LICENSE_LIST) >= 0)
		{
			//TODO:TBD:registered on all workspace object? or on Item/ItemRevision/Dataset seperately?
			REGISTER_EVENT_HANDLER("WorkspaceObject", NULL, MESSAGE_LICENSE_DETACH, METHOD_post_action_type, nxl_ada_license_handler);
			REGISTER_EVENT_HANDLER("WorkspaceObject", NULL, MESSAGE_LICENSE_ATTACH, METHOD_post_action_type, nxl_ada_license_handler);
			//REGISTER_EVENT_HANDLER("WorkspaceObject", NULL, "TC_detach_ada_license", METHOD_pre_action_type, nxl_ada_license_handler);
			//REGISTER_EVENT_HANDLER("WorkspaceObject", NULL, "TC_attach_ada_license", METHOD_pre_action_type, nxl_ada_license_handler);
		}
	}
	return ITK_ok;
}