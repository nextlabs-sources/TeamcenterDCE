#include "dce_mem.h"
#include "dce_translator.h"
#include "dce_log.h"
#include "utils_string.h"
#include <ae/dataset.h>
#include <tccore/item.h>
#include <tccore/grm.h>
#include <tccore/grmtype.h>
#include <tccore/aom_prop.h>
#include <res/res_itk.h>
#include <tc/preferences.h>
#include <nxl_action_handler.h>
#include <nxl_secure_data.h>
#include <key_value_list.h>
#include <hashtable.h>

//Prototypes
int sync_item_revision(tag_info_ro revInfo, tag_list_p context);
int sync_dataset(tag_info_ro dsInfo, tag_info_ro revInfo, tag_list_p context);


static tag_t get_relation_attaches()
{
	static tag_t relAttaches = NULLTAG;
	if(relAttaches == NULLTAG)
	{
		ITK_EVAL(GRM_find_relation_type(TC_attaches_rtype, &relAttaches));
		DBGLOG("Relation-'"TC_attaches_rtype"'='%u'", relAttaches);
	}
	return relAttaches;
}
static logical dataset_is_attaches_relation(tag_info_ro dsInfo)
{
	if(dsInfo != NULL
		&& dsInfo->rootType == type_dataset)
	{
		tag_t attRel = get_relation_attaches();
		if(attRel != NULLTAG)
		{
			tag_t relation = NULLTAG;
			tag_info_ro revInfo = tag_info_get_parent(dsInfo);
			if(revInfo != NULL
				&& ITK_EVAL(GRM_find_relation(revInfo->tagId, dsInfo->tagId, attRel, &relation)))
			{
				return relation != NULLTAG;
			}
		}
	}
	return false;
}
typedef BOOL (*pfSnapshotOperation)(tag_info_ro tagInfo);

BOOL res_check(tag_t objTag)
{
	BOOL valid = TRUE;
	tag_t resTag = NULLTAG;
	if(ITK_EVAL(RES_ask_reservation_object(objTag, &resTag)))
	{
		char *str = NULL;
		logical bl;
		int i;
		char *changeid = NULL;
		char *reason = NULL;
		if(ITK_EVAL(RES_ask_change_id2(resTag, &changeid)))
		{
			DBGLOG("Reservation-%u:ChangeId='%s'", resTag, changeid);
		}
		if(ITK_EVAL(RES_ask_reason2(resTag, &reason)))
		{
			DBGLOG("Reservation-%u:reason='%s'", resTag, reason);
		}
		if(changeid != NULL && reason != NULL)
		{
			char *combinedId;
			int size = strlen(changeid) + 1 + strlen(reason);
			string_list_p filter = pref_get_snapshot_bypass_changeids();
			TC_ALLOCN(combinedId, char, size+1);
			sprintf_s(combinedId, size+1, "%s|%s", changeid, reason);
			DBGLOG("Matching...'%s'", combinedId);
			for(i=0; i<filter->count; i++)
			{
				if(string_wildcmp(combinedId, filter->items[i]))
				{
					DBGLOG("'Matched-'%s'", filter->items[i]);
					valid = FALSE;
					break;
				}
			}
			TC_FREE(combinedId);
			string_list_free(filter);
		}
		TC_FREE(changeid);
		TC_FREE(reason);
		if(ITK_EVAL(RES_ask_export_on_checkout(resTag, &bl)))
		{
			DBGLOG("Reservation-%u:Export=%d", resTag, bl);
		}
		if(ITK_EVAL(RES_ask_exp_directory2(resTag, &str)))
		{
			DBGLOG("Reservation-%u:ExportDirectory='%s'", resTag, str);
			TC_FREE(str);
		}
		if(ITK_EVAL( RES_ask_reservation_type(resTag, &i)))
		{
			DBGLOG("Reservation-%u:ResType=%d", resTag, i);
		}
		if(ITK_EVAL( RES_ask_reservation_state(resTag, &i)))
		{
			DBGLOG("Reservation-%u:ResState=%d", resTag, i);
		}
	}
	return valid;
}
int nxl_debug(METHOD_message_t *msg, va_list args)
{
	int ifail = ITK_ok;

	TC_HANDLER_START;

	DEBUG_MSG(msg);

	TC_HANDLER_END;
	return ifail;
}
int nxl_checkout_handler( METHOD_message_t *msg, va_list args )
{
	int ifail = ITK_ok;
	tag_t objTag = NULLTAG;

	TC_HANDLER_START;
	
	DEBUG_MSG(msg);
	/********************/
	/* Initialization	*/
	/********************/
	//Get the parameters from the RES_checkin_msg
	//Parameters:
	//	tag_t  objectType (I)  
	if((objTag = va_arg(args, tag_t)) != NULLTAG)
	{
		pfSnapshotOperation action = NULL;
		tag_info_p tagInfo = tag_info_new(objTag);
		const char *messageName = GET_MSGNAME(msg);
		DBGLOG("Checking Out:"TAG_INFO_FMT, TAG_INFO_ARGS(tagInfo));
		//check whether the message is checkout or cancel checkout
		if(messageName != NULL)
		{
			if(strcmp(messageName, RES_cancel_checkout_msg) == 0)
			{
				action = snapshot_clean;
			}
			else if(strcmp(messageName, RES_checkout_msg) == 0
				&& pref_enabled_snapshot_on_checkout())
			{
				char *userId = NULL;
				string_list_p users = pref_get_snapshot_bypass_users();
				ITK_CALL(POM_get_user_id(&userId));
				NEED_FREE(userId);
				DBGLOG("Current User='%s'", userId);
				if(string_list_index_of(users, userId) < 0)
				{
					if(res_check(objTag))
					{
						action = snapshot_capture;
					}
				}
				string_list_free(users);
				TC_FREE(userId);
			}
		}
		if(action == NULL) goto FINISH;
		//GUIDELINE:always taking snapshot on the checked out object regardless the preference
		//for Item check out, only taking snapshot on Item only
		//for ItemRevision check out, taking snapshot on ItemRevision and children datasets which has relation TC_Attaches
		//		because those datasets are checked out together with this ItemRevision but no checkout event triggerreed
		//for Dataset check out, taking snapshot if it's a non-TC_Attaches dataset, otherwise skipped because the snapshot has been taken during ItemRevision checkout
		if(tagInfo->rootType == type_dataset
			&& dataset_is_attaches_relation(tagInfo))
		{
			DBGLOG("The checking out dataset has the relation-TC_Attaches with ItemRevision, skipped taking snapshot.");
		}
		else
		{
			//take/clean snapshot
			action(tagInfo);
		}
		if(tagInfo->rootType == type_item_revision)
		{
			tag_t relAttaches = get_relation_attaches();
			DBGLOG("Searching children datasets which has relation-'"TC_attaches_rtype"'(%u)...", relAttaches);
			//Take snapshot on children dataset with TC_Attaches relation
			if(relAttaches != NULLTAG)
			{
				int cntSecObj;
				tag_t *listSecObj;
				//get children objects from ItemRevision
				if(ITK_EVAL(GRM_list_secondary_objects_only(tagInfo->tagId, relAttaches, &cntSecObj, &listSecObj)))
				{
					int iSecObj;
					NEED_FREE(listSecObj);
					for(iSecObj = 0; iSecObj < cntSecObj; iSecObj++)
					{
						tag_info_p dsInfo = tag_info_new(listSecObj[iSecObj]);
						DBGLOG(TAG_INFO_FMT":"TC_attaches_rtype"[%d/%d]-"TAG_INFO_FMT, TAG_INFO_ARGS(tagInfo), iSecObj+1, cntSecObj, TAG_INFO_ARGS(dsInfo));
						if(dsInfo->rootType == type_dataset)
						{
							//take/clean snapshot on dataset
							action(dsInfo);
						}
						CALL_DTL(tag_info_free(dsInfo));
					}//End for(iSecObj = 0; iSecObj < cntSecObj; iSecObj++)

					TC_FREE(listSecObj);
				}
			}
		}
FINISH:
		tag_info_free(tagInfo);
	}

	TC_HANDLER_END;
	return ifail;
}
int nxl_checkin_handler( METHOD_message_t *msg, va_list args )
{
	int ifail = ITK_ok;
	tag_t objTag = NULLTAG;

	TC_HANDLER_START;
	
	DEBUG_MSG(msg);
	/********************/
	/* Initialization */
	/********************/
	//Get the parameters from the RES_checkin_msg
	//Parameters:
	//	tag_t  objectType (I)  
	if((objTag = va_arg(args, tag_t)) != NULLTAG)
	{
		int synced = 0;
		tag_info_p tagInfo = tag_info_new(objTag);
		//TODO:make this context available in current thread
		//TBD:after skip dataset checkin, don't need to share it in thread level
		tag_list_p context = TAG_LIST_new();
		DBGLOG(TAG_INFO_FMT":CheckedIn", TAG_INFO_ARGS(tagInfo));
		switch(tagInfo->rootType)
		{
			case type_dataset:
				if(dataset_is_attaches_relation(tagInfo))
				{
					DBGLOG("this dataset which has TC_Attaches relation to ItemRevision has been processed in the POST checkin event of parent ItemRevision");
				}
				else
				{
					//dataset is not descendant of DocumentRevision, sync this dataset
					synced = sync_dataset(tagInfo, NULL, context);
				}
				break;
			case type_item:
				if(pref_get_revision_propagation())
				{
					//check preference if item checkin should be propagated
					if(pref_get_item_propagation())
					{
						COMPARE_RESULT result;
						DTLLOG("Compaing properties with snapshot...");
						if(!snapshot_compare_properties(tagInfo, &result)
							|| COMPARE_NEED_PROPAGATE(result))
						{
							int cntRevs;
							tag_t *listRevs;
							//get children ItemRevisions from Item
							if(ITK_EVAL(ITEM_find_revisions (tagInfo->tagId, "*", &cntRevs, &listRevs))
								&& cntRevs > 0)
							{
								int iRev;
								NEED_FREE(listRevs);
								for(iRev = 0; iRev < cntRevs; iRev++)
								{
									tag_info_p revInfo = tag_info_new(listRevs[iRev]);
									if(revInfo->rootType==type_item_revision)
									{
										logical isCheckout = false;
										//skip itemrevision is still checking out
										ITK_EVAL(RES_is_checked_out(revInfo->tagId, &isCheckout));
										DBGLOG(TAG_INFO_FMT":Revision[%d/%d]-"TAG_INFO_FMT" checkedOut=%d", TAG_INFO_ARGS(tagInfo), iRev+1, cntRevs, TAG_INFO_ARGS(revInfo), isCheckout);
										if(!isCheckout)
										{
											//proces item revision without checking n
											synced = sync_item_revision(revInfo, context);
										}
									}
									else
									{
										DBGLOG(TAG_INFO_FMT":Revision[%d/%d]-"TAG_INFO_FMT, TAG_INFO_ARGS(tagInfo), iRev+1, cntRevs, TAG_INFO_ARGS(revInfo));
									}
									CALL_DTL(tag_info_free(revInfo));
								}//End for(iSecObj = 0; iSecObj < cntSecObj; iSecObj++)

								TC_FREE(listRevs);
							}
							else
							{
								DBGLOG(TAG_INFO_FMT":has 0 child ItemRevision", TAG_INFO_ARGS(tagInfo));
							}
						}
						else
						{
							DBGLOG("Propagation is not required");
						}
					}
					else
					{
						DBGLOG("Item Checkin Propagation is DISABLED");
					}
				}
				else
				{
					DBGLOG("ItemRevision Checkin Propagation is DISABLED, no need propagate Item Checkin!");
				}
				break;
			case type_item_revision:
				synced = sync_item_revision(tagInfo, context);
				break;
			default:
				break;
		}
		snapshot_clean(tagInfo);
		CALL_DTL(tag_info_free(tagInfo));
		DBGLOG("Processed %d datasets and Synced %d datasets", context->count, synced);
		TAG_LIST_free(context);
	}

	TC_HANDLER_END;
	return ifail;
}

logical dataset_need_autoprotect(tag_info_ro tagInfo)
{
	logical ret = FALSE;
	if(tagInfo!=NULL && tagInfo->rootType == type_dataset)
	{
		char *datasetType = NULL;
		if(ITK_EVAL(WSOM_ask_object_type2(tagInfo->tagId, &datasetType)))
		{
			const char *enabled = NULL;
			NEED_FREE(datasetType);
			DBGLOG("DatasetType='%s'", datasetType);
			if(datasetType!=NULL
				&& (enabled = pref_get_char(PREF_AUTOPROTECT_ENABLED, AUTOPROTECT_DISABLED)) != NULL)
			{
				if(_stricmp(enabled, AUTOPROTECT_ALL)==0)
				{
					//all dataset types should be auto protected
					ret = TRUE;
				}
				else if(_stricmp(enabled, AUTOPROTECT_ENABLED)==0)
				{
					//check the white list of auto protect
					int ntypes = 0;
					char **types = NULL;
					if(ITK_EVAL(PREF_ask_char_values(PREF_AUTOPROTECT_DATASETS, &ntypes, &types)))
					{
						int i;
						NEED_FREE(types);
						DBGLOG(PREF_AUTOPROTECT_DATASETS" has %d values", ntypes);
						for(i=0; i<ntypes; i++)
						{
							DBGLOG("==>[%d/%d]=%s", i+1, ntypes, types[i]);
							if(_stricmp(types[i], datasetType)==0)
							{
								ret = TRUE;
								break;
							}
						}
						TC_FREE(types);
					}
				}//else if(_stricmp(enabled, AUTOPROTECT_ENABLED)==0)
			}//if(pref_get_char(PREF_AUTOPROTECT_ENABLED
			TC_FREE(datasetType);
		}//if(ITK_EVAL(WSOM_ask_object_type2(tagInfo->tagId, &datasetType)))
	}//if(tagInfo!=NULL && tagInfo->rootType == type_dataset)
	return ret;
}

int sync_dataset(tag_info_ro dsInfo, tag_info_ro revInfo, tag_list_p ctx)
{
	tag_t request = NULLTAG;
	if(dsInfo != NULL
		&&dsInfo->rootType == type_dataset)
	{
		request_type syncOp = request_non;
		request_priority priority = priority_low;
		const char *reason = NULL;
		ref_status_t fileStatus = {0};
		if(TAG_LIST_contains(ctx, dsInfo->tagId))
		{
			DBGLOG(TAG_INFO_FMT" has been synced in this session!", TAG_INFO_ARGS(dsInfo));
			return 0;
		}
		//check if all the named refs are nxl files
		fileStatus = dataset_verify_ref_status(dsInfo, TRUE, TRUE);
		//remove protection
		if((fileStatus.nNxl > 0)
			&& ((get_solution_info())->type==solution_inMotion))
		{
			COMPARE_RESULT compareResult;
			if(!snapshot_compare_files(dsInfo, &compareResult)
				|| COMPARE_IS_DIFFERENT(compareResult))
			{
				DBGLOG(TAG_INFO_FMT" need to be unprotected!", TAG_INFO_ARGS(dsInfo));
				//InMotion mode, all decrypt files should be removed protection
				syncOp = request_unprotect;
				reason = "unprotect on checkin";
				//unprotect doesn't require any arguments
				priority = priority_high;
			}
		}
		else if (dataset_is_nxl(dsInfo))
		{
			//reprotect
			if ((fileStatus.nNonNxl > 0)
				&& dataset_need_autoprotect(dsInfo))
			{
				DBGLOG(TAG_INFO_FMT" need to be reprotected!", TAG_INFO_ARGS(dsInfo));
				syncOp = request_protect;
				reason = "reprotect on checkin";
				priority = priority_high;
			}
			//updateTag
			else if (fileStatus.nNxl > 0
				&& pref_metadtasync_enabled()
				&& res_check(dsInfo->tagId))
			{
				COMPARE_RESULT result;
				//send translation request to update tag when
				//V-A:if any named reference is nxl file
				//X-B:if all named references are nxl file
				//This dataset need to be update tags in following scenarios
				//1, snpashot doesn't exist which may means force update
				//2, snapshot shows that the key property value is changed
				if (!snapshot_compare_properties(dsInfo, &result)
					|| COMPARE_IS_DIFFERENT(result))
				{
					DBGLOG(TAG_INFO_FMT" need to be updated!", TAG_INFO_ARGS(dsInfo));
					reason = "updateTags on checkin";
					priority = priority_medium;
					syncOp = request_update;
				}
			}
		}
		else
		{
			//secure non-protected dataset
			if (fileStatus.nNonNxl > 0)
			{
				request = secure_dataset(dsInfo, "checkin");
				goto PROCESSED;
			}
		}
		if(syncOp == request_non)
		{
			DBGLOG(TAG_INFO_FMT" DONT need to be synced!", TAG_INFO_ARGS(dsInfo));
			goto PROCESSED;
		}
		if(revInfo == NULL)
		{
			revInfo = tag_info_get_parent(dsInfo);
		}
		//sent translation request
		CALL_DBG(request = send_translation_request(dsInfo, revInfo, NULL, 0, syncOp, priority, reason));
		
PROCESSED:
		CALL_DBG(TAG_LIST_add(ctx, dsInfo->tagId));
		//remove value of backup file
		snapshot_clean(dsInfo);
	}
	return request == NULLTAG ? 0 : 1;
}

int sync_item_revision(tag_info_ro revInfo, tag_list_p ctx)
{
	int synced = 0;
	if(revInfo!=NULL
		&& revInfo->rootType == type_item_revision)
	{
		int cntSecObj, iSecObj;
		GRM_relation_t *secObjs;
		//get children objects from ItemRevision
		if(ITK_EVAL(GRM_list_secondary_objects(revInfo->tagId, NULLTAG, &cntSecObj, &secObjs))
			&& cntSecObj > 0)
		{
			tag_t attachesRelation = get_relation_attaches();
			BOOL propagate = FALSE;
			NEED_FREE(secObjs);
			if(pref_get_revision_propagation())
			{
				COMPARE_RESULT result;
				BOOL success = snapshot_compare_properties(revInfo, &result);
				DBGLOG(TAG_INFO_FMT":PerferenceEnabled=TRUE HasSnapshot=%d CompareResult=%u", TAG_INFO_ARGS(revInfo), cntSecObj, success, result);
				propagate = !success || COMPARE_NEED_PROPAGATE(result);
			}
			else
			{
				propagate = FALSE;
				DBGLOG(TAG_INFO_FMT":PerferenceEnabled=FALSE NeedPropagate=%d", TAG_INFO_ARGS(revInfo), propagate);
			}
			for(iSecObj = 0; iSecObj < cntSecObj; iSecObj++)
			{
				tag_info_p dsInfo = tag_info_new(secObjs[iSecObj].secondary);
				DBGLOG(TAG_INFO_FMT":SecObj[%d/%d]-"TAG_INFO_FMT"relationType='%u'"
					,TAG_INFO_ARGS(revInfo), iSecObj+1, cntSecObj, TAG_INFO_ARGS(dsInfo), secObjs[iSecObj].relation_type);
				if(dsInfo->rootType == type_dataset)
				{
					if(secObjs[iSecObj].relation_type == attachesRelation)
					{
						//for TC_Attaches dataset, always sync them
						synced += sync_dataset(dsInfo, revInfo, ctx);
					}
					else if(propagate)
					{
						//for other dataset, sync them when ItemRevision propagation is enabled AND
						//ItemRevision doesn't have snapshot which mean this is a force update
						//OR ItemRevision have snapshot and has changes on propagation properties
						logical isCheckout = false;
						//skip dataset which is still checking out in force
						ITK_EVAL(RES_is_checked_out(dsInfo->tagId, &isCheckout));
						if(!isCheckout)
						{
							synced += sync_dataset(dsInfo, revInfo, ctx);
						}
						else
						{
							DBGLOG(TAG_INFO_FMT":Dataset is checked out now, will be sync after manual checkedin", TAG_INFO_ARGS(dsInfo));
						}
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