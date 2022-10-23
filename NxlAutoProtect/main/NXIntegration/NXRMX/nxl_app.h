/*
	The functions prototypes that will be used for UI related 
*/
#ifndef NXL_NX_USAGE_CONTROL_APP_H_INCLUDED
#define NXL_NX_USAGE_CONTROL_APP_H_INCLUDED
#include <uf_mb.h>

#define APP_NAME "NXL_NXRMX"
#define NX_RMX_DIALOG_TITLE "NextLabs RMX"

/*
    Debug Helper
*/
#define PRINT_BUTTON(btn) \
if(btn != NULL)	\
{	\
	NXDBG("Button Name:%s", btn->name);	\
	NXDBG("Button ID:%d", btn->id);	\
	NXDBG("Button App ID:%d", btn->app_id);	\
	NXDBG("Button Menubar Name:%s", btn->menubar_name);	\
	NXDBG("Button Num of Ancestors:%d", btn->num_ancestors);	\
	NXDBG("Button Type:%c", btn->type);	\
}

#ifdef __cplusplus
extern "C"
{
#endif

//nx start up
DllExport void ufsta( char *param, int *returnCode, int rlen );

//
//App Actions
//
#define APP_ACTION_ABOUT "NXL_NXRMX_about"
UF_MB_cb_status_t action_about(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );

UF_MB_cb_status_t action_debug(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );

#define APP_ACTION_LOG_BUTTON "NXL_NXRMX_log_button"
UF_MB_cb_status_t action_log_button(
	UF_MB_widget_t             widget,
	UF_MB_data_t               client_data,
	UF_MB_activated_button_p_t call_button);

#define APP_ACTION_BLOCKER "NXL_NXRMX_blocker"
UF_MB_cb_status_t action_blocker(
	UF_MB_widget_t             widget,
	UF_MB_data_t               client_data,
	UF_MB_activated_button_p_t call_button);

#define APP_ACTION_FLCONFIRMER "NXL_NXRMX_force_load_confirmer"
UF_MB_cb_status_t action_force_load_confirmer(
	UF_MB_widget_t             widget,
	UF_MB_data_t               client_data,
	UF_MB_activated_button_p_t call_button);


#define APP_ACTION_VIEW_WORKPART_INFO "NXL_NXRMX_view_workpart_info"
UF_MB_cb_status_t action_view_workpart_info(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button);

#define APP_ACTION_PROTECT_WORKPART "NXL_NXRMX_protect_workpart"
UF_MB_cb_status_t action_protect_workpart(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button);

#define APP_ACTION_PROTECT_WORKPART_CHILDREN "NXL_NXRMX_protect_workpart_children"
UF_MB_cb_status_t action_protect_workpart_children(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button);

#define APP_ACTION_PROTECT_DISPLAYEDPART "NXL_NXRMX_protect_displayedpart"
UF_MB_cb_status_t action_protect_displayedpart(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button);
/*
#define APP_ACTION_TEST "NXL_APP_test"
UF_MB_cb_status_t action_test(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );

#define APP_ACTION_LIST_PARTS "NXL_APP_list_parts"
UF_MB_cb_status_t action_list_parts(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );

#define APP_ACTION_LIST_COMPS "NXL_APP_list_comps"
UF_MB_cb_status_t action_list_comps(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );
	*/

#ifdef __cplusplus
}
#endif

/*
    Debug Helper
*/
#define PRINT_BUTTON(btn) \
if(btn != NULL)	\
{	\
	NXDBG("Button Name:%s", btn->name);	\
	NXDBG("Button ID:%d", btn->id);	\
	NXDBG("Button App ID:%d", btn->app_id);	\
	NXDBG("Button Menubar Name:%s", btn->menubar_name);	\
	NXDBG("Button Num of Ancestors:%d", btn->num_ancestors);	\
	NXDBG("Button Type:%c", btn->type);	\
}

#endif