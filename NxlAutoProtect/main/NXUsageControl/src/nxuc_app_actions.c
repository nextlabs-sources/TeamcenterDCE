#include <uf_mb.h>
#include <nxuc_app.h>
#include <nx_utils.h>
#include <nx_evaluator.h>

/*
	Debug Helper
*/
#define PRINT_BUTTON(btn) \
{	\
	NXDBG("Button Name:%s", btn->name);	\
	NXDBG("Button ID:%d", btn->id);	\
	NXDBG("Button App ID:%d", btn->app_id);	\
	NXDBG("Button Menubar Name:%s", btn->menubar_name);	\
	NXDBG("Button Num of Ancestors:%d", btn->num_ancestors);	\
	NXDBG("Button Type:%c", btn->type);	\
}
/*----------------------------------------------------------------------------*
 *  NXUC_APP_about
 *
 *      Show Nextlabs App Version Information
 *----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_about(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button )
{
	NXDBG("=========================================");

    if( NX_EVAL(UF_initialize()) ) 
    {
#define ABOUT_LINES 4
		int nlines = ABOUT_LINES;
		int iline = 0;
		char *about[ABOUT_LINES];
		/* Place code to enter application here */	
		//char *btnTName = NULL;
		PRINT_BUTTON(call_button);
		about[iline++] = "====================================";
		about[iline++] = "  NX Usage Control by Nextlabs.Inc";
		about[iline++] = "  Version "VER_FILE_STR;
		//TODO: revise about info
		//about[iline++] = "* (This is a POC version for RMC team)";
		about[iline++] = "====================================";
		nx_dialog_show_lines(about, nlines);
		NX_CALL(UF_terminate());
	}

	NXDBG("Ends here." );
    return UF_MB_CB_CONTINUE;
}

/*----------------------------------------------------------------------------*
 *  NXUC_APP_evaluate_workpart_only
 *
 *      Evaluate user action by button for workpart only
 *----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_evaluate_workpart_only(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button )
{
    UF_MB_cb_status_t cb_status = UF_MB_CB_CONTINUE;
	NXDBG("=========================================");
	
    if( NX_EVAL(UF_initialize()) )
	{
		const char *actionName = call_button->name;
		PRINT_BUTTON(call_button);
		if(actionName != NULL)
		{
			if(!nx_evaluate(actionName, TRUE))
			{
				cb_status = UF_MB_CB_CANCEL;
			}
		}
		NX_EVAL(UF_terminate());
	}

	NXDBG("Ends here(status=%d)", cb_status );
    return( cb_status );
}
/*----------------------------------------------------------------------------*
 *  action_evaluate_edit_workpart
 *
 *      Evaluate user action for edit operations
 *----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_evaluate_edit_workpart(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button )
{
    UF_MB_cb_status_t cb_status = UF_MB_CB_CONTINUE;
	NXDBG("=========================================");
	
    if( NX_EVAL(UF_initialize()) )
	{
		const char *actionName = call_button->name;
		PRINT_BUTTON(call_button);
		if(actionName != NULL)
		{
			if(!nx_evaluate(RIGHT_EDIT, TRUE))
			{
				cb_status = UF_MB_CB_CANCEL;
			}
		}
		NX_EVAL(UF_terminate());
	}

	NXDBG("Ends here(status=%d)", cb_status );
    return( cb_status );
}


/*----------------------------------------------------------------------------*
 *  action_evaluate_edit_assembly
 *
 *      Evaluate user action for edit operations
 *----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_evaluate_edit_assembly(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button )
{
    UF_MB_cb_status_t cb_status = UF_MB_CB_CONTINUE;
	NXDBG("=========================================");
	
    if( NX_EVAL(UF_initialize()) )
	{
		const char *actionName = call_button->name;
		PRINT_BUTTON(call_button);
		if(actionName != NULL)
		{
			if(!nx_evaluate(RIGHT_EDIT, FALSE))
			{
				cb_status = UF_MB_CB_CANCEL;
			}
		}
		NX_EVAL(UF_terminate());
	}

	NXDBG("Ends here(status=%d)", cb_status );
    return( cb_status );
}
/*----------------------------------------------------------------------------*
 *  action_evaluate_print_assembly
 *
 *      Evaluate user action for print operations
 *----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_evaluate_print_assembly(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button )
{
    UF_MB_cb_status_t cb_status = UF_MB_CB_CONTINUE;
	NXDBG("=========================================");
	
    if( NX_EVAL(UF_initialize()) )
	{
		const char *actionName = call_button->name;
		PRINT_BUTTON(call_button);
		if(actionName != NULL)
		{
			if(!nx_evaluate(RIGHT_PRINT, FALSE))
			{
				cb_status = UF_MB_CB_CANCEL;
			}
		}
		NX_EVAL(UF_terminate());
	}

	NXDBG("Ends here(status=%d)", cb_status );
    return( cb_status );
}
/*----------------------------------------------------------------------------*
 *  action_evaluate_saveas_workpart
 *
 *      Evaluate user action for save as operations
 *----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_evaluate_saveas_workpart(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button )
{
    UF_MB_cb_status_t cb_status = UF_MB_CB_CONTINUE;
	NXDBG("=========================================");
	
    if( NX_EVAL(UF_initialize()) )
	{
		const char *actionName = call_button->name;
		PRINT_BUTTON(call_button);
		if(actionName != NULL)
		{
			if(!nx_evaluate(RIGHT_SAVEAS, TRUE))
			{
				cb_status = UF_MB_CB_CANCEL;
			}
		}
		NX_EVAL(UF_terminate());
	}

	NXDBG("Ends here(status=%d)", cb_status );
    return( cb_status );
}
/*----------------------------------------------------------------------------*
 *  action_evaluate_saveas_assembly
 *
 *      Evaluate user action for send operations
 *----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_evaluate_saveas_assembly(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button )
{
    UF_MB_cb_status_t cb_status = UF_MB_CB_CONTINUE;
	NXDBG("=========================================");
	
    if( NX_EVAL(UF_initialize()) )
	{
		const char *actionName = call_button->name;
		PRINT_BUTTON(call_button);
		if(actionName != NULL)
		{
			if(!nx_evaluate(RIGHT_SAVEAS, TRUE))
			{
				cb_status = UF_MB_CB_CANCEL;
			}
		}
		NX_EVAL(UF_terminate());
	}

	NXDBG("Ends here(status=%d)", cb_status );
    return( cb_status );
}
/*----------------------------------------------------------------------------*
 *  NXUC_APP_evaluate_clipboard
 *
 *      Evaluate user action for copy/cut to clipboard operations
 *----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_evaluate_clipboard_workpart(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button )
{
    UF_MB_cb_status_t cb_status = UF_MB_CB_CONTINUE;
	NXDBG("=========================================");
	
    if( NX_EVAL(UF_initialize()) )
	{
		const char *actionName = call_button->name;
		PRINT_BUTTON(call_button);
		if(actionName != NULL)
		{
			if(!nx_evaluate(RIGHT_CLIPBOARD, TRUE))
			{
				cb_status = UF_MB_CB_CANCEL;
			}
		}
		NX_EVAL(UF_terminate());
	}

	NXDBG("Ends here(status=%d)", cb_status );
    return( cb_status );
}

/*----------------------------------------------------------------------------*
 *  NXUC_APP_evaluate_all_parts
 *
 *      Evaluate user action by button for all parts
 *----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_evaluate_all_parts(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button )
{
    UF_MB_cb_status_t cb_status = UF_MB_CB_CONTINUE;
	NXDBG("=========================================");
	
    if( NX_EVAL(UF_initialize()) )
	{
		const char *actionName = call_button->name;
		PRINT_BUTTON(call_button);
		if(actionName != NULL)
		{
			if(!nx_evaluate(actionName, FALSE))
			{
				cb_status = UF_MB_CB_CANCEL;
			}
		}
		NX_EVAL(UF_terminate());
	}

	NXDBG("Ends here(status=%d)", cb_status );
    return( cb_status );
}