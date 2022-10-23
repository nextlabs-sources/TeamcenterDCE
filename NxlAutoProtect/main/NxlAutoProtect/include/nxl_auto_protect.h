/*
	This header file defines the APIs which will be called from Teamcenter logic
	For example workflow action handler, object/property event handler
	These APIs must be registered in NxlAutoProtect_register.c
*/
#ifndef NXL_AUTO_PROTECT_H_INCLUDED
#define NXL_AUTO_PROTECT_H_INCLUDED
#include "nxl_auto_protect_exports.h"
#include <res/res_itk.h>
#include <ae/dataset_msg.h>

//Feature - Workflow Action Handler
#define ACTION_HANDLER_NAME ( "NextLabs-Integrated-EDRM" ) //used in workflow designer
#define ACTION_HANDLER_DESC ( "Send translation request when the dataset meets the NXL evaluation" )
extern DCEAPI int nxl_auto_protect_handler(EPM_action_message_t msg); //definition for workflow handlers

//Feature - DLPApplicationHook
#define DLP_EXIT_NAME "IMF_dlp_classification_code"
extern DCEAPI int nxl_add_classification_code( int *decision, va_list args );

//Event Handlers
extern DCEAPI int nxl_checkout_handler( METHOD_message_t *msg, va_list args );
extern DCEAPI int nxl_checkin_handler( METHOD_message_t *msg, va_list args );
extern DCEAPI int nxl_debug(METHOD_message_t *msg, va_list args);
extern DCEAPI int Nxl3_dataset_save_post(METHOD_message_t* msg, va_list args);


extern DCEAPI int metadatasync_on_license_changed();
extern DCEAPI int metadatasync_to_dataset_on_import();
extern DCEAPI int refresh_duid_install();
#endif //NXL_AUTO_PROTECT_H_INCLUDED