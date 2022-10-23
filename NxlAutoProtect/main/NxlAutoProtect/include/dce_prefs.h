#ifndef NXL_DCE_PREFERENCES_H_INCLUDED
#define NXL_DCE_PREFERENCES_H_INCLUDED
#include <key_value_list.h>
#include <utils_string_list.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *pref_get_char(const char *key, const char *defaultValue);
const char *pref_get_char_cached(const char *key, const char *defaultValue);
BOOL pref_get_logical(const char *key, BOOL defaultValue);
extern DCEAPI string_list_p pref_get_values(const char *key);

#ifdef __cplusplus
}
#endif

#define REG_DCE_KEY "SOFTWARE\\NextLabs\\TeamcenterRMX"
/*
	Solution related
	Cached because should not change
*/
//TODO:review these names
#define PREF_SOLUTION_TYPE "NXL_RMX_Solution"
#define SOLUTION_AT_REST "AtRest"
#define SOLUTION_IN_MOTION "InMotion"
//Components:TBD:not configurable in current version
#define PREF_ENABLED_COMPONENTS "NXL_RMX_Enabled_Components"
#define COMPONENT_DLP_SERVER_EXIT "DLPServerExit"
#define COMPONENT_WF_HANDLER "WorkflowHandler"
#define COMPONENT_CHECKIN_HANDLER "CheckinHandler"
#define COMPONENT_SET_TOOLUSED "SetToolUsed"

/*
	Tool related
*/
//#define PREF_NXL_TOOL "NXL_Tool_Name"
#define NXL_TOOL_NAME "Nxl3_FileHandler"

/*
	Is NextLabs Protected
*/
#define pref_update_protected_on_upload() pref_get_logical("NXL_Update_IsProtected_On_Upload", TRUE)

/*
	Validate File Name on imported
	Default = True
*/
#define pref_get_validate_name_on_imported() pref_get_logical("NXL_Validate_Name_on_Imported", TRUE)
#define PREF_VALIDATION_DISABLED_DATASETS "NXL_Protection_Validation_Disabled_Dataset_Types"
extern BOOL dataset_need_protection_validation(const char *dsType);

/*
	Evaluation related
	Cached because rarely change
*/
#define PREF_REMOTE_IP "NXL_Remote_IP"
#define REG_REMOTE_IP "JavaPCHost"
const char *get_remote_ip();
#define PREF_EVALUATION_ATTRS "NXL_Evaluation_Attributes"

/*
	Translation request related
*/
#define pref_send_props_for_protect() pref_get_logical("NXL_Send_Properties_For_Protect", FALSE)
#define pref_send_props_for_update() pref_get_logical("NXL_Send_Properties_For_Update", FALSE)

/*
	Checkin Propagation related
	NonCached
*/
#define PREF_PROPAGATION_ITEM "NXL_Item_Checkin_Propagation_enabled"
#define PREF_PROPAGATION_REVISION "NXL_ItemRevision_Checkin_Propagation_enabled"
#define pref_get_item_propagation() pref_get_logical(PREF_PROPAGATION_ITEM, FALSE)
#define pref_get_revision_propagation() pref_get_logical(PREF_PROPAGATION_REVISION, TRUE)

/*
	Introduce a new preference to allow user enable/disable sending translation request for update tag.
	Rename the existing preference NXL_Checkout_Snapshot_enabled to NXL_Snapshot_enabled to allow two versions of library running in parallel
	The default value of the new preference is disabled while the default value of old one is enabled(Bajaj has imported the default value into Teamcenter)
	
	This change is request by Bajaj, because the snapshot is potentially causing problem, and metadata sync is not needed for them
*/
#define pref_metadtasync_enabled() pref_get_logical("NXL_Metadatasync_enabled", FALSE)
#define pref_enabled_snapshot_on_checkout() pref_get_logical("NXL_Snapshot_enabled", FALSE)
//deprecated, used for old libraries
//#define pref_enabled_snapshot_on_checkout() pref_get_logical("NXL_Checkout_Snapshot_enabled", TRUE)
#define pref_get_snapshot_bypass_users() pref_get_values("NXL_Snapshot_Bypass_Users")
#define pref_get_snapshot_bypass_changeids() pref_get_values("NXL_Snapshot_Bypass_ChangeIds");
#define PREF_SNAPSHOT_PROPERTIES "NXL_Snapshot_Properties"

/*
	Auto Reprotect related
	ENABLED is non-cached
	Dataset Types is not cached that doesn't require restart RAC
*/
#define PREF_AUTOPROTECT_ENABLED "NXL_AUTOPROTECT_ENABLED"
#define AUTOPROTECT_ENABLED "true"
#define AUTOPROTECT_DISABLED "false"
#define AUTOPROTECT_ALL "all"

#define PREF_AUTOPROTECT_DATASETS "NXL_AUTOPROTECT_DATASET_TYPES"
//logical dataset_need_autoprotect(tag_info_ro tagInfo)

#define PREF_PROTECT_ATTRS "NXL_Transferred_Attributes"
kvl_p pref_get_protect_attributes();//non-cached

/*
	DLP related
	Cached for performance
	Case insensitive
*/
#define PREF_DLP_DATASETS_WHITE_LIST "NXL_DLP_Dataset_Type_White_List"
//BOOL is_dataset_filtered(tag_t dsTag)
#define PREF_DLP_FILES_WHITE_LIST "NXL_DLP_File_Extension_White_List"
//BOOL is_file_filtered(tag_info_p fileInfo)
#define PREF_DLP_DISABLE_EVALUATION "NXL_DLP_Evaluation_Disabled"
//BOOL is_evaluation_enabled();
#define PREF_REFERENCES_NEED_RECTIFY "NXL_Text_References"

/*
	Cached because should not change
*/
#define PREF_PROPERTIES_BLACK_LIST "NXL_Property_Black_List"

/*
	not cached
*/
#define pref_nr_blacklist() pref_get_values("NXL_NR_Blacklist")

//format <datasetType>|<referenceName>
#define pref_nr_auto_delete_on_conflict() pref_get_values("NXL_NR_Auto_Deletion")
BOOL is_reference_require_auto_deletion(const char *dsType, const char *referenceName);


//
#define pref_nr_metadatasyn_on_import() pref_get_values("NXL_NR_Metadata_Sync_on_Import")
BOOL is_reference_require_metadatasync(const char *dsType, const char *referenceName);
/*
	Workflow Filters
	non-cached
*/
#define PREF_RELATION_FILTER "NXL_Relation_Filter"

/*WholeFile download*/
#define PREF_WHOLE_FILE_DOWNLOAD_ENABLED() pref_get_logical("NXL_Whole_File_Download_enabled", false)
#endif //NXL_DCE_PREFERENCES_H_INCLUDED