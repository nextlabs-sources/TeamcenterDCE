/*
		Teamcenter related utilites
*/
#include <nxl_auto_protect_exports.h>
#include <epm/epm.h>
#include <unidefs.h>
#include <time.h>
#include "hashtable.h"
#include "dce_prefs.h"
#include <utils_string_list.h>

#ifndef NXL_DCE_TC_UTILS_H_INCLUDED
#define NXL_DCE_TC_UTILS_H_INCLUDED

#define NXL_SUFFIX ".nxl"

#ifdef __cplusplus
extern "C" {
#endif
//
//Debug & Log related
//
extern DCEAPI logical report_error(const char *file, int line, const char *func, const char *call, int ret, logical exit_on_error);//TRUE=no error
#define IFERR_ABORT(X)  (report_error( __FILE__, __LINE__, __FUNCTION__, #X, X, TRUE))
#define ITK_EVAL(X) (report_error( __FILE__, __LINE__, __FUNCTION__, #X, X, FALSE))
extern DCEAPI void error_list();

#define ITK_CALL(x) CALL_DTL(ITK_EVAL(x))

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(p)
#endif

extern DCEAPI int set_protection_mark();
extern DCEAPI void clear_protection_mark();

#define TC_HANDLER_START	\
	SCOPE_START;	\
	error_list();	\
	set_protection_mark();

#define TC_HANDLER_END	\
	DBGLOG("****************%s**** finalizing*********", __FUNCTION__);	\
	error_list();	\
	DBGLOG("Clearing all errors...");	\
	EMH_clear_errors();	\
	clear_protection_mark();	\
	SCOPE_END

typedef enum _obj_type
{
	type_item_revision = 0,
	type_dataset = 1,
	type_file = 2,
	type_item = 3,
	type_unknown = 4
}obj_type;

tag_t get_class_id(obj_type type);
extern DCEAPI const char *get_class_name(obj_type type);

//
//tag_info_s structure definition
//
typedef struct tag_info_s
{
	tag_t tagId;
	tag_t classId;
	char *className;
	char *name;
	obj_type rootType;
	void *reserved;	//dont' tempt! reserved for internal use only!!
} *tag_info_p;
typedef const struct tag_info_s *tag_info_ro;//read only tag_info

#define TAG_INFO_FMT "%s{Tag=%u Class='%s'(%u) Name='%s'}"
#define TAG_INFO_ARGS(ti) get_class_name(ti->rootType), ti->tagId, ti->className, ti->classId, ti->name

#define PRINT_TAG_INFO(ti)	\
if( ti != NULL)	\
{	\
	DBGLOG(#ti "->" TAG_INFO_FMT, TAG_INFO_ARGS(ti));	\
}	\
else	\
{	\
	DBGLOG(#ti " == NULL");	\
}

//tag_info_p related methods
extern DCEAPI tag_info_p tag_info_new(const tag_t tagdata);
extern DCEAPI void tag_info_free(tag_info_p tagInfo);
hashtable_ro tag_info_get_props(tag_info_ro tagInfo);
tag_info_ro tag_info_get_parent(tag_info_ro tagInfo);
void tag_info_debug_lock(tag_info_ro tagInfo);

//
//TAG_LIST module
//
typedef struct tag_list_s
{
	tag_t *tags;
	int count;
} *tag_list_p;
tag_list_p TAG_LIST_new();
logical TAG_LIST_contains(tag_list_p list, tag_t tag);
void TAG_LIST_add(tag_list_p list, tag_t tag);
void TAG_LIST_free(tag_list_p list);

//
//Workflow ARGS module
//
typedef enum wf_operation_e 
{
	wfop_undefined,
	wfop_protect,
	wfop_unprotect
}wf_operation_t;
typedef struct _action_handler_args
{
	string_list_p datasetTypes;
	time_t scheduledTime;
	tag_list_p relationTypes;
	string_list_p relationNames;
	int process_assembly;//three state, 0=not initialized, 1=true, -1=false
	int skip_nxl_datasets;
	wf_operation_t operation;
	const char *workflowName;
	//context
	tag_list_p attachments;
	tag_list_p datasets;
	tag_list_p itemRevisions;
	tag_list_p translations;
} *wf_args_p;
wf_args_p WF_ARGS_new();
void WF_ARGS_free(wf_args_p args);

//
//Snapshot Module
//
BOOL snapshot_exist(tag_info_ro tagInfo);
BOOL snapshot_capture(tag_info_ro tagInfo);
BOOL snapshot_clean(tag_info_ro tagInfo);
typedef unsigned int COMPARE_RESULT;
#define COMPARE_IS_DIFFERENT(r) ((r) > 0)
#define COMPARE_NEED_PROPAGATE(r) ((r) & 2)
//Return whether compare sucess
BOOL snapshot_compare_properties(tag_info_ro tagInfo, COMPARE_RESULT *result);
BOOL snapshot_compare_files(tag_info_ro datasetInfo, COMPARE_RESULT *result);


//
//dataset info related utilities
//
//ask the named reference information of specific dataset
int dataset_list_ref_infos(tag_info_ro dsInfo, BOOL includeAll, tag_info_p **nrTagInfos);
extern DCEAPI BOOL dataset_find_nr_info(tag_info_ro dsInfo, const char *rName, int rType, const char *fileName, tag_info_p *fileInfo);
extern DCEAPI logical dataset_is_nxl(tag_info_ro dsInfo);
extern DCEAPI BOOL dataset_set_tool(tag_info_ro dsInfo, BOOL nxlTool);
extern DCEAPI BOOL dataset_set_is_protected(tag_info_ro dsInfo, logical newValue);
extern DCEAPI kvl_p dataset_get_protect_attributes(tag_info_ro dsInfo);
extern DCEAPI char *dataset_get_type(tag_info_ro dsInfo);
BOOL dataset_update_attr_value(tag_info_ro dsInfo, const char *attrName, const char *newValue, BOOL overrideOnExist);
int dataset_delete_refs(tag_info_ro dsInfo, tag_list_p tags, string_list_ro refNames);
typedef struct ref_status_s
{
	int nTotal;
	int nNxl;
	int nNonNxl;
}ref_status_t;
 ref_status_t dataset_verify_ref_status(tag_info_ro dsInfo, BOOL readFile, BOOL updateTool);
//
//file info related utillities
//
extern DCEAPI BOOL imanfile_is_protected_fsc(tag_info_ro fileInfo, BOOL *isProtected);
BOOL imanfile_read_metadata_fsc(tag_info_ro fileInfo, kvl_p *tags);
//logical imanfile_rename(tag_info_ro fileInfo, char *newName);
BOOL imanfile_validate_name(tag_info_ro fileInfo, char **outputName, BOOL *isProtected);
tag_t imanfile_search_dataset(tag_info_ro fileInfo);
BOOL imf_delete_file(tag_t fileToBeDeleted);

BOOL ask_reference_name(tag_t fileTag, tag_t dsTag, char **referenceName);
int find_relation_types(char * const *inputNames, int ninputs, tag_list_p relationTypes, string_list_p relationNames);
logical is_nxl_dataset_tag(tag_t tag);
const char *get_tc_bin();

logical tag_info_ask_prop_value(tag_info_ro obj, const char *propName, char **oPropValue);

typedef struct EnumNRCallbackParams_s {
	const char* refName;
	int refType;
	tag_t refTag;
	BOOL isblacklisted;
	void* cbData;
} EnumNRCallbackParams_t, * EnumNRCallbackParams_p;
typedef BOOL (*EnumNRCallback)(EnumNRCallbackParams_t *params);
void EnumerateDatasetReferences(tag_t dsTag, EnumNRCallback cb, void * cbData);

typedef struct EnumerateCallbackParams_s {
	tag_t itemTag;
	void* cbData;
} EnumerateCallbackParams_t, * EnumerateCallbackParams_p;
typedef BOOL(*EnumerateCallback)(EnumerateCallbackParams_p params);
void EnumerateItemRevDatasets(tag_t revTag, EnumerateCallback cb, void *cbData);

//
//Solution module
//
typedef enum solution_e
{
	solution_non = 0,
	solution_atRest = 1,
	solution_inMotion = 3
}solution_type;
typedef enum component_e
{
	no_component = 0,
	dlp_exit_comp = 1,
	wf_handler_comp = 2,
	set_tool_comp = 4,
	checkin_handler_comp = 8
}component_t;
typedef struct solution_info_s
{
	solution_type type;
	component_t components;
}const *solution_info;

solution_info get_solution_info();


#ifdef __cplusplus
}
#endif
#endif //NXL_DCE_TC_UTILS_H_INCLUDED