/*
	The functions prototypes that will be used for UI related 
*/
#ifndef NXL_NX_USAGE_CONTROL_APP_H_INCLUDED
#define NXL_NX_USAGE_CONTROL_APP_H_INCLUDED
#include <uf_mb.h>

#define APP_NAME "NXUC_APP"

#ifdef __cplusplus
extern "C"
{
#endif

//nx start up
DllExport void ufsta( char *param, int *returnCode, int rlen );

int nx_evaluate(const char *action, int isWorkpartOnly);

//
//App Actions
//
#define APP_ACTION_ABOUT "NXUC_APP_about"
UF_MB_cb_status_t action_about(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );

UF_MB_cb_status_t action_evaluate_edit_workpart(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );

UF_MB_cb_status_t action_evaluate_edit_assembly(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );

UF_MB_cb_status_t action_evaluate_print_assembly(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );

UF_MB_cb_status_t action_evaluate_saveas_workpart(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );

UF_MB_cb_status_t action_evaluate_saveas_assembly(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );

UF_MB_cb_status_t action_evaluate_clipboard_workpart(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );

#define APP_ACTION_EVALUATE_ALL "NXUC_APP_evaluate_all_parts"
UF_MB_cb_status_t action_evaluate_all_parts(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );

#define APP_ACTION_EVALUATE_WORKPART "NXUC_APP_evaluate_workpart_only"
UF_MB_cb_status_t action_evaluate_workpart_only(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button );


#ifdef __cplusplus
}
#endif

#endif