#include <uf_mb.h>
#include "nxl_app.h"
#include "nx_utils.h"
#include "nx_contexts.h"
#include <uf_modl.h>
#include <uf_part.h>
#include <uf_obj.h>
#include <uf_assem.h>

#include <NXRMX/PartContext.h>
#include <hook/hook_defs.h>
#include <hook/libugui.h>
#include <hook/libsyss.h>
#include <hook/libugui.h>
#include <hook/libsyss.h>

#include <utils_string_list.h>

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

int nx_rmx_message_box(UF_UI_MESSAGE_DIALOG_TYPE type, string_list_ro messages)
{
	int res;
	NX_CALL(UF_UI_message_dialog(NX_RMX_DIALOG_TITLE, type, messages->items, messages->count, false, NULL, &res));
	return res;
}
/*----------------------------------------------------------------------------*
 *  NXL_APP_about
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
		/* Place code to enter application here */	
		//char *btnTName = NULL;
		PRINT_BUTTON(call_button);
		static string_list_p messages = nullptr;
		if (messages == nullptr)
		{
			messages = string_list_new();
			string_list_add(messages, "====================================");
			string_list_add(messages, "  NX Rights Management eXtension by NextLabs.Inc");
			string_list_add(messages, "  Version " FILE_VER);
			//TODO: revise about info
			//about[iline++] = "* (This is a POC version for RMC team)";
			string_list_add(messages, "====================================");
		}
		nx_rmx_message_box(UF_UI_MESSAGE_INFORMATION, messages);
		NX_CALL(UF_terminate());
	}

	NXDBG("Ends here." );
    return UF_MB_CB_CONTINUE;
}
void callback_log_button_id(int reason, void const * pBtnId, void * pCtx)
{
	NXDBG("Reason=%d, btnId=%d arg2=%p", reason, pBtnId==NULL?0:*((int*)pBtnId), pCtx);
	if (pBtnId == NULL)
		return;
	int btnId = *((int*)pBtnId);
	const char *btnName = MB_ask_button_name(btnId);
	NXMSG("Button id=%d Name='%s'", btnId, btnName);	
}
UF_MB_cb_status_t action_log_button(
	UF_MB_widget_t             widget,
	UF_MB_data_t               client_data,
	UF_MB_activated_button_p_t call_button)
{
	NXDBG("=========================================");

	if (NX_EVAL(UF_initialize()))
	{
		static CPP_PTR funcPtr = NULL;
		PRINT_BUTTON(call_button);
		UF_MB_state_t state;
		if (funcPtr != NULL)
		{
			CALLBACK_remove_function(funcPtr);
			funcPtr = NULL;
		}
		if (NX_EVAL(UF_MB_ask_toggle_state(call_button->id, &state)))
		{
			NXDBG("Button-'%s' state=%d", call_button->name, state);
			if (state == UF_MB_ON) {
				//install pre action
				funcPtr = MB_add_pre_action_callback(callback_log_button_id, NULL);
			}
		}

		NX_CALL(UF_terminate());
	}

	NXDBG("Ends here.");
	return UF_MB_CB_CONTINUE;

}

/*----------------------------------------------------------------------------*
 *  NXL_APP_debug
 *
 *----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_debug(
    UF_MB_widget_t             widget,
    UF_MB_data_t               client_data,
    UF_MB_activated_button_p_t call_button )
{
	NXDBG("=========================================");

    if( NX_EVAL(UF_initialize()) ) 
	{
		/*list_reason();
		MB_ask_num_menubars();
		auto mainmenu = MB_ask_main_menubar();
		auto menuname = MB_ask_menubar_name_by_mbar(mainmenu);
		auto blocker = MB_ask_action_by_name("NXL_NXRMX_blocker",98);*/
		/*
		static char cbArg[255] = "callback args";
		DBGLOG("cbarg=%p", &cbArg);
		const void *ret = MB_add_pre_action_callback(action_callback, &cbArg);

		DBGLOG("MB_add_pre_action_callback() returns %p", ret);
		*/
		NX_CALL(UF_terminate());
	}

	NXDBG("Ends here." );
    return UF_MB_CB_CONTINUE;
}
extern bool confirm_whole_file_download(const std::vector<tag_t> &parts);

UF_MB_cb_status_t action_force_load_confirmer(
	UF_MB_widget_t             widget,
	UF_MB_data_t               client_data,
	UF_MB_activated_button_p_t call_button)
{
	UF_MB_cb_status_t ret = UF_MB_CB_CONTINUE;
	NXDBG("=========================================");
	if (NX_EVAL(UF_initialize()))
	{
		PRINT_BUTTON(call_button);
		//if any file of displed part is minimally loaded, ask user to confirm
		auto parts = part_list_display_parts();
		if (!confirm_whole_file_download(parts))
		{
			//cancel this operation
			ret = UF_MB_CB_CANCEL;
		}
	}

	NXDBG("Ends here.");
	return ret;
}
/*----------------------------------------------------------------------------*
*  NXL_NXRMX_blocker
*
*----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_blocker(
	UF_MB_widget_t             widget,
	UF_MB_data_t               client_data,
	UF_MB_activated_button_p_t call_button)
{
	NXDBG("=========================================");
	UF_MB_cb_status_t ret = UF_MB_CB_CONTINUE;
	if (NX_EVAL(UF_initialize()))
	{
		PRINT_BUTTON(call_button);
		//if any file of displed part is protected file, cancel this operation
		auto parts = part_list_display_parts();
		int nprotectedParts = 0;
		string_list_p messages = string_list_new();
		string_list_add(messages, "This operation is blocked as following parts are protected:");
		string_list_add(messages, " ");
		for (auto p = parts.begin(); p != parts.end(); p++) {
			auto pContext = PartContext::RetrieveContext(*p);
			if (pContext == nullptr)
			{
				NXDBG("Part-%u: ContextNotFound", *p);
				continue;
			}
			NXDBG(CONTEXT_FMT, CONTEXT_FMT_ARGS(pContext));
			if (pContext->is_protected()) {
				nprotectedParts++;
				string_list_add(messages, name_to_display(pContext->part_name()).c_str());
			}
		}
		if (nprotectedParts > 0)
		{
			string_list_add(messages, " ");
			string_list_add(messages, "Please refer to your administrator for details.");
			//cancel this operation
			ret = UF_MB_CB_CANCEL;
			nx_rmx_message_box(UF_UI_MESSAGE_WARNING, messages);
		}
		string_list_free(messages);
	}

	NXDBG("Ends here.");
	return ret;
}
/*----------------------------------------------------------------------------*
*  NXL_NXRMX_view_workpart_info
*
*----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_view_workpart_info(
	UF_MB_widget_t             widget,
	UF_MB_data_t               client_data,
	UF_MB_activated_button_p_t call_button)
{
	NXDBG("=========================================");
	UF_MB_cb_status_t ret = UF_MB_CB_CONTINUE;
	if (NX_EVAL(UF_initialize()))
	{
		PRINT_BUTTON(call_button);
		tag_t workpart = UF_ASSEM_ask_work_part();
		auto name = tag_to_name(workpart);
		auto file = NxlPath(name_to_file(name.c_str()));
		auto displayName = NxlString(name_to_display(name.c_str()));
		if (file.isempty()) {
			//in case part file is not returned, try to search from contexts
			file = NxlPath(PartContext::SearchFileByName(name.c_str()));
		}
		if (!file.isempty())
		{
			NXDBG("showing file info dialog('%s', '%s')...", file.tchars(), displayName.tchars());
			ShowFileInfoDialog(file.wstr(), displayName.wchars());
		}
	}

	NXDBG("Ends here.");
	return ret;
}
#define RESPONSE_OK 1
#define RESPONSE_CANCEL 0
#define RESPONSE_OTHER -1
static bool confirm_protect_parts(const std::vector<PartContext*>& parts) {
	NXDBG("%d parts are going to be protected", parts.size());
	if (!parts.empty())
	{
		int response = RESPONSE_OTHER;
		UF_UI_message_buttons_t buttons = { true, false, true, "Yes", NULL, "No", RESPONSE_OK, RESPONSE_OTHER, RESPONSE_CANCEL };
		string_list_p messages = string_list_new();
		//TODO:review this message
		string_list_add(messages, "Following files are going to be converted into NextLabs protected format:");
		string_list_add(messages, "");
		for (auto i = parts.begin(); i != parts.end(); i++)
		{
			string_list_add(messages, (*i)->part_file().chars());
		}
		string_list_add(messages, "");
		string_list_add(messages, "Do you want continue?");
		NX_CALL(UF_UI_message_dialog(NX_RMX_DIALOG_TITLE, UF_UI_MESSAGE_QUESTION, messages->items, messages->count, false, &buttons, &response));
		NXDBG("User selected=%d", response);
		string_list_free(messages);
		return response == RESPONSE_OK;
	}
	return false;
}
static bool confirm_rpm_folder(const std::vector<PartContext*>& parts) {
	NXDBG("%d parts are not in rpm folder", parts.size());
	if (!parts.empty())
	{
		int response = RESPONSE_OTHER;
		UF_UI_message_buttons_t buttons = { true, false, true, "Yes", NULL, "No", RESPONSE_OK, RESPONSE_OTHER, RESPONSE_CANCEL };
		string_list_p messages = string_list_new();
		//TODO:review this message
		string_list_add(messages, "Following files are currenly not in NextLabs RPM Folder yet:");
		string_list_add(messages, "");
		for (auto i = parts.begin(); i != parts.end(); i++)
		{
			string_list_add(messages, (*i)->part_file().chars());
		}
		string_list_add(messages, "");
		string_list_add(messages, "Do you want add the folder automatically?");
		NX_CALL(UF_UI_message_dialog(NX_RMX_DIALOG_TITLE, UF_UI_MESSAGE_QUESTION, messages->items, messages->count, false, &buttons, &response));
		NXDBG("User selected=%d", response);
		string_list_free(messages);
		return response == RESPONSE_OK;
	}
	return true;
}
static bool confirm_modified_parts(const std::vector<PartContext*>& parts) {
	NXDBG("%d parts are modified", parts.size());
	if (!parts.empty())
	{
		int response = RESPONSE_OTHER;
		UF_UI_message_buttons_t buttons = { true, false, true, "Yes", NULL, "No", RESPONSE_OK, RESPONSE_OTHER, RESPONSE_CANCEL };
		string_list_p messages = string_list_new();
		//TODO:review this message
		string_list_add(messages, "Following files have been modified in current session:");
		string_list_add(messages, "");
		for (auto i = parts.begin(); i != parts.end(); i++)
		{
			auto name = tag_to_name((*i)->part_tag());
			string_list_add(messages, name_to_display(name.c_str()).c_str());
		}
		string_list_add(messages, "");
		string_list_add(messages, "Do you allow us to save them before the protection?");
		NX_CALL(UF_UI_message_dialog(NX_RMX_DIALOG_TITLE, UF_UI_MESSAGE_QUESTION, messages->items, messages->count, false, &buttons, &response));
		NXDBG("User selected=%d", response);
		string_list_free(messages);
		return response == RESPONSE_OK;
	}
	return true;
}
static std::vector<PartContext*>  getPartContextForProtect(tag_t part, bool includeChilren)
{
	std::vector<PartContext*> partContexts;
	if (nx_isugmr()) {
		NXWAR("user cannot protect part in managed");
	}
	else if(part != NULLTAG)
	{
		std::vector<tag_t> parts;
		if (includeChilren)
		{
			parts = assembly_list_parts(part);
		}
		else
		{
			parts.push_back(part);
		}
		std::vector<tag_t> partsNotProtected;
		for (auto itag = parts.begin(); itag != parts.end(); itag++) {
			PartContext* pContext = PartContext::RetrieveContext(*itag);
			if (pContext != nullptr) {
				NXDBG(CONTEXT_FMT, CONTEXT_FMT_ARGS(pContext));
				if (!pContext->is_protected())
				{
					pContext->RefreshPartName();
					pContext->RefreshPartFile();
					partsNotProtected.push_back((*itag));
				}
			}
		}
		//build context collection after all parts have been loaded
		//otherwise the pointer may be invalid
		if (!partsNotProtected.empty()) {
			for (auto itag = partsNotProtected.begin(); itag != partsNotProtected.end(); itag++) {
				PartContext* pContext = PartContext::FindByTag2(*itag);
				if (pContext != nullptr) {
					NXDBG(CONTEXT_FMT, CONTEXT_FMT_ARGS(pContext));
					partContexts.push_back(pContext);
				}
			}
		}
	}
	return partContexts;
}
static bool save_parts(tag_t part, bool includeChilren)
{
	int error = ERROR_OK;
	UF_ASSEM_work_part_context_p_t context = NULL;
	if (UF_ASSEM_ask_work_part() != part) {
		NXDBG("Original Work part=%d inputPart=%d", UF_ASSEM_ask_work_part(), part);
		if (!NX_EVAL(UF_ASSEM_set_work_part_context_quietly(part, &context)))
		{
			return false;
		}
		NXDBG("Work part=%d", UF_ASSEM_ask_work_part());
	}
	if (includeChilren)
	{
		NXDBG("saving work part and the children...");
		error = NX_CALL(UF_PART_save());
	}
	else
	{
		NXDBG("saving work part...");
		error = NX_CALL(UF_PART_save_work_only());
	}
	if (context != NULL) {
		NXDBG("Restoring work part...")
		if (NX_EVAL(UF_ASSEM_restore_work_part_context_quietly(&context)))
		{
			//TBD
		}
		NXDBG("Work part=%d", UF_ASSEM_ask_work_part());
	}
	if (error!= ERROR_OK)
	{
		string_list_p errorMessages = string_list_new();
		char errMsg[133];
		UF_get_fail_message(error, errMsg);
		string_list_add(errorMessages, "Saving part failed with below error:");
		string_list_add(errorMessages, errMsg);
		string_list_add(errorMessages, "Please save them manually and try again.");
		nx_rmx_message_box(UF_UI_MESSAGE_ERROR, errorMessages);
		return false;
	}
	return true;

}
class ProtectReport {
public:
	void Show() {
		//ensure listing window is opened
		logical windowOpening = false;
		if (!NX_EVAL(UF_UI_is_listing_window_open(&windowOpening))
			|| !windowOpening)
		{
			//if eval failed or window is not opening, force open it
			NX_CALL(UF_UI_open_listing_window());
		}
		_opening = true;
		for (auto i = _messages.begin(); i != _messages.end(); i++)
		{
			UF_UI_write_listing_window(i->c_str());
		}
		_messages.clear();
	}
	void WriteMessage(const char* msg) {
		if (_opening)
		{
			UF_UI_write_listing_window(msg);
		}
		else
		{
			_messages.push_back(msg);
		}
	}
	void Reset()
	{
		UF_UI_exit_listing_window();
		_opening = false;
		_messages.clear();
	}
private:
	logical _opening;
	std::vector<std::string> _messages;
};
char messageBuffer[1024];
ProtectReport s_report;
#define WRITE_REPORT(fmt, ...) {sprintf(messageBuffer, fmt "\n", ##__VA_ARGS__);s_report.WriteMessage(messageBuffer);}
#define NEW_REPORT() s_report.Reset();
#define SHOW_REPORT() s_report.Show();

static int protect_part(tag_t part, bool includeChilren)
{
	int nprotected = 0;
	NxlMetadataCollection tags;
	std::vector<PartContext*> modifiedParts;
	//step 1: search part which is not protected
	std::vector<PartContext*> partsNotProtected = getPartContextForProtect(part, includeChilren);
	if (partsNotProtected.empty())
		return nprotected;
	//step 2:check if any part is modified
	for (auto iContext = partsNotProtected.begin(); iContext != partsNotProtected.end(); iContext++) {
		//check modified
		if (UF_PART_is_modified((*iContext)->part_tag()))
		{
			NXDBG(CONTEXT_FMT " is modified", CONTEXT_FMT_ARGS(*iContext));
			modifiedParts.push_back(*iContext);
		}
	}
	if (!confirm_modified_parts(modifiedParts))
	{
		return nprotected;
	}
	if (!modifiedParts.empty()
		&& !save_parts(part, includeChilren)) {
		return nprotected;
	}
	//step 3: select tags
	NXDBG("showing protect dialog...");
	if (!ShowProtectDialog(partsNotProtected.front()->part_file().wstr(), tags))
	{
		return nprotected;
	}
	NXDBG("tags='%s'", tags.str());
	//if (confirm_protect_parts(partsNotProtected))
	NEW_REPORT();
	{
		//
		int ipart = 1;
		int npart = partsNotProtected.size();
		bool isProtected;
		RPMSession::RPMFolderStatus folderStatus;
		SHOW_REPORT();
		for (auto i = partsNotProtected.begin(); i != partsNotProtected.end(); i++, ipart++) {
			//protect file one by one
			auto file = (*i)->part_file();
			NXDBG("Processing-%d/%d '%s'...", ipart, npart, file.chars());
			WRITE_REPORT("Protecting '%s'", file.chars());
			//skip if still modified
			if (UF_PART_is_modified((*i)->part_tag()))
			{
				NXWAR("somehow the file is still modified, skip protection");
				WRITE_REPORT("\tFile is modified! skipped");
				continue;
			}
			//add rpm folder if needed
			auto error = g_rpmSession->CheckFileStatus(file.wstr(), &isProtected, &folderStatus);
			if (error != ERROR_SUCCESS)
			{
				NXERR("CheckFileStatusFailed('%s'):'%s'", file.tstr(), g_rpmSession->GetLastError().Message());
				WRITE_REPORT("\tERROR:%s", g_rpmSession->GetLastError().Message());
				continue;
			}
			if (isProtected)
			{
				NXERR("FileAlreadyProtected('%s'):'%s'", file.tstr(), g_rpmSession->GetLastError().Message());
				WRITE_REPORT("\tFile is already protected!");
				continue;
			}
			if (!RPMSession::FolderStatus_IsInRpm(folderStatus)) {
				NXDBG("'%s':file is not in a rpm folder", file.tstr());
				//TODO:add rpm folder
				NxlPath folder = file.RemoveFileSpec();
				if ((error = g_rpmSession->AddRPMFolder(folder.wchars())) != ERROR_SUCCESS) {
					NXERR("AddRPMFolderFailed('%s'):'%s'", file.tstr(), g_rpmSession->GetLastError().Message());
					WRITE_REPORT("\tAdd RPM Folder failed:'%s'", folder.chars());
					continue;
				}
				WRITE_REPORT("\tAdded New RPM folder:'%s'", folder.chars());
			}

			error = g_rpmSession->ProtectFile(file.wstr(), tags, true);
			switch (error.Code())
			{
			case ERROR_SUCCESS:
				NXSYS("Protected:'%s'", file.tstr());
				WRITE_REPORT("\tProtected");
				nprotected++;
				{
					bool hasviewright;
					//check view right;
					if (g_rpmSession->CheckFileRights(file.wstr(), "RIGHT_VIEW", hasviewright, false) == 0
						&&!hasviewright)
					{
						//close part
						NX_CALL(UF_PART_close((*i)->part_tag(), 0, 2));
						WRITE_REPORT("\tNoViewRight, part is closed");
					}
					else
					{
						NXDBG("Reloading...");
						auto context = PartContext::Reload((*i)->part_tag());
						if (!context->is_protected()) {
							//if the context is failed to updated, we manually mark context as protected
							//in the future, it may be reprotect on save or export
							context->SetProtectionStatus(true, tags);
						}
					}
				}
				break;
			default:
				//TODO:??
				NXERR("ProtectFailed('%s'):'%s'", file.tstr(), g_rpmSession->GetLastError().Message());
				WRITE_REPORT("\tProtectFailed:%s", g_rpmSession->GetLastError().Message());
				break;
			}
		}
	}
	NXDBG("protected %d of %d non-protected parts", nprotected, partsNotProtected.size());
	WRITE_REPORT("Protected %d files", nprotected);
	if (nprotected > 0) {
		//refresh overlay
		//TODO:use PartContext::RegisterContextChangedHandler()
		overlay_force_refresh();
		UpdateButtonSensitivity();
	}
	WRITE_REPORT("Finished");
	return nprotected;

}
/*----------------------------------------------------------------------------*
*  NXL_NXRMX_protect_workpart
*
*----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_protect_workpart(
	UF_MB_widget_t             widget,
	UF_MB_data_t               client_data,
	UF_MB_activated_button_p_t call_button)
{
	NXDBG("=========================================");
	UF_MB_cb_status_t ret = UF_MB_CB_CONTINUE;
	if (NX_EVAL(UF_initialize()))
	{
		PRINT_BUTTON(call_button);
		protect_part(UF_ASSEM_ask_work_part(), false);
	}

	NXDBG("Ends here.");
	return ret;
}
/*----------------------------------------------------------------------------*
*  NXL_NXRMX_protect_workpart_children
*
*----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_protect_workpart_children(
	UF_MB_widget_t             widget,
	UF_MB_data_t               client_data,
	UF_MB_activated_button_p_t call_button)
{
	NXDBG("=========================================");
	UF_MB_cb_status_t ret = UF_MB_CB_CONTINUE;
	if (NX_EVAL(UF_initialize()))
	{
		PRINT_BUTTON(call_button);
		protect_part(UF_ASSEM_ask_work_part(), true);
	}

	NXDBG("Ends here.");
	return ret;
}
/*----------------------------------------------------------------------------*
*  NXL_NXRMX_protect_displayedpart
*
*----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_protect_displayedpart(
	UF_MB_widget_t             widget,
	UF_MB_data_t               client_data,
	UF_MB_activated_button_p_t call_button)
{
	NXDBG("=========================================");
	UF_MB_cb_status_t ret = UF_MB_CB_CONTINUE;
	if (NX_EVAL(UF_initialize()))
	{
		PRINT_BUTTON(call_button);
		protect_part(UF_PART_ask_display_part(), true);
	}

	NXDBG("Ends here.");
	return ret;
}