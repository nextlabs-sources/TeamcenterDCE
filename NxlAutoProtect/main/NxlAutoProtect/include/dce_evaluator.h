#ifndef NXL_DCE_EVALUATOR_H_INCLUDED
#define NXL_DCE_EVALUATOR_H_INCLUDED
#include "tc_utils.h"
#include <nxl_evaluator_defs.h>

//Evaluation
#define DO_TRANSLATION 1
#define DO_DEFAULT 0
#define SKIP_TRANSLATION -1

//EDRM Evaluation
#define DRM_EVAL_OPERATION "RIGHT_EXECUTE_WORKFLOW"//ver2.0, In ver1.5 it's "RIGHT_PROTECT"
#define DRM_OB_NAME "TCDRM"
#define TAGGING_KEY_TYPE "Tagging"
#define TAGGING_KEY_COLUMN_NAME "Name"
#define TAGGING_KEY_COLUMN_VALUE "Value"

int evaluate_internal(const char *action, tag_info_ro dsInfo, tag_info_ro revInfo, kvl_ro additionalProps, obligation_list *obligations, int *nobligation);

int evaluate_workflow_protect(tag_info_ro dsInfo, tag_info_ro revInfo, kvl_p tagMap);

int evaluate_workflow_unprotect(const char *wfName, tag_info_ro dsInfo, tag_info_ro revInfo);

int evaluate_download(tag_info_ro dsInfo, tag_info_ro revInfo);


#endif//NXL_DCE_EVALUATOR_H_INCLUDED