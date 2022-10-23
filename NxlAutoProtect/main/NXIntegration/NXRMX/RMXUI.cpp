#include <NXRMX/nxl_app.h>
#include <uf_mb.h>
#include <uf_assem.h>
#include <stdafx.h>
#include <NXRMX/nx_utils.h>
#include <NXRMX/nx_contexts.h>

namespace RMX
{
	class UIButton {
	public:
		UIButton(const char* name) :_id(0) {
			_name[0] = '\0';
			if (name != NULL)
				strcpy(_name, name);
		}
		int get_id() {
			if (_id <= 0)
			{
				NX_CALL(UF_MB_ask_button_id(_name, &_id));
			}
			return _id;
		}
		bool set_sensitivity(bool s) {
			bool ret = NX_EVAL(UF_MB_set_button_sensitivity(get_id(), s ? UF_MB_ON : UF_MB_OFF));
			NXDBG("Set Button-%s(%d) Sensitivity to %d", _name, _id, s);
			return ret;
		}
	private:
		int _id;
		char _name[MAX_PATH];
	};
	namespace Buttons {
		static UIButton ProtectWorkPart = UIButton("NXRMX_BUTTON_PROTECT_WORKPART");
		static UIButton ProtectWorkPartChildren = UIButton("NXRMX_BUTTON_PROTECT_WORKPART_CHILDREN");
		static UIButton ProtectDisplayedPart = UIButton("NXRMX_BUTTON_PROTECT_ASSEMBLY");
		static UIButton ViwFileInfo = UIButton("NXRMX_BUTTON_VIEW_FILEINFO");
	}
}

#define REGISTER_PART_EVENT_HANDLER(reason, cb, retFuncId) \
if(NX_EVAL(UF_add_callback_function(reason, cb, #reason, &retFuncId))){	\
	NXDBG("UF_add_callback_function(%d-"#reason"):%p", reason, retFuncId);	\
}
void UpdateButtonSensitivity() {
	tag_t workPart = UF_ASSEM_ask_work_part();
	bool managedMode = nx_isugmr();
	//disable all buttons by default
	RMX::Buttons::ViwFileInfo.set_sensitivity(false);
	RMX::Buttons::ProtectWorkPart.set_sensitivity(false);
	RMX::Buttons::ProtectWorkPartChildren.set_sensitivity(false);
	RMX::Buttons::ProtectDisplayedPart.set_sensitivity(false);
	if (nx_isugmr())
	{
		//managed mode
		const PartContext* pContext = PartContext::FindByTag(workPart);
		if (pContext == nullptr) {
			//minimal loaded
		}
		else if(pContext->is_protected())
		{
			//full loaded and is protected
			RMX::Buttons::ViwFileInfo.set_sensitivity(true);
		}
		else
		{
			//full loaded and not protected
		}
	}
	else
	{
		const PartContext* pContext = PartContext::RetrieveContext(workPart);
		if (pContext == nullptr) {
			//unknown status
		}
		else if (pContext->is_protected())
		{
			//work part is protected
			RMX::Buttons::ViwFileInfo.set_sensitivity(true);
			//check if any other prt in displayed part is non protected
			auto displayedPart = part_list_display_parts();
			for (auto ipart = displayedPart.begin(); ipart != displayedPart.end(); ipart++) {
				if (*ipart == workPart)
					continue;
				pContext = PartContext::RetrieveContext(*ipart);
				if (!pContext->is_protected()) {
					//if any part in displayed part is not protectd, enable protect assembly
					RMX::Buttons::ProtectDisplayedPart.set_sensitivity(true);
					break;
				}
			}
			//check children part of work part
			auto children = assembly_list_parts(workPart);
			for (auto ipart = children.begin(); ipart != children.end(); ipart++) {
				if (*ipart == workPart)
					continue;
				pContext = PartContext::RetrieveContext(*ipart);
				if (!pContext->is_protected()) {
					//if any part in children of work part is not protectd, enable protect work part and assembly
					RMX::Buttons::ProtectWorkPartChildren.set_sensitivity(true);
					break;
				}
			}
		}
		else
		{
			//work part is not protected
			RMX::Buttons::ProtectWorkPart.set_sensitivity(true);
			RMX::Buttons::ProtectWorkPartChildren.set_sensitivity(true);
			RMX::Buttons::ProtectDisplayedPart.set_sensitivity(true);
		}

	}
}
void RMX_UI_on_workpart_changed(UF_callback_reason_e_t reason, const void* pTag, void* closure)
{
	if (NX_EVAL(UF_initialize()))
	{
		tag_t partTag = *((tag_t*)pTag);
		if (reason == UF_change_work_part_reason)
		{
			UpdateButtonSensitivity();
		}
		NX_EVAL(UF_terminate());
	}

}

bool install_rmxui_items()
{
	UF_MB_action_t actionTable[] =
	{
	 { APP_ACTION_ABOUT, action_about, NULL },
	 { APP_ACTION_BLOCKER, action_blocker, NULL },
	 { APP_ACTION_FLCONFIRMER, action_force_load_confirmer, NULL },
	 { APP_ACTION_LOG_BUTTON, action_log_button, NULL },
	 { APP_ACTION_VIEW_WORKPART_INFO, action_view_workpart_info, NULL },
	 { APP_ACTION_PROTECT_WORKPART, action_protect_workpart, NULL },
	 { APP_ACTION_PROTECT_WORKPART_CHILDREN, action_protect_workpart_children, NULL },
	 { APP_ACTION_PROTECT_DISPLAYEDPART, action_protect_displayedpart, NULL },
	 //{ APP_ACTION_PRESAVE, action_preSave, NULL },
	 //{ APP_ACTION_POSTSAVE, action_postSave, NULL },
	 //{ APP_ACTION_RESET, action_reset, NULL },
	 //{ "NXL_APP_export", action_export, NULL },
	 { "NXL_APP_debug", action_debug, NULL },
	 //{ APP_ACTION_LIST_COMPS, action_list_comps, NULL },
	 { NULL, NULL, NULL }	//action table must end with this item
	};
	//register actions
	NX_CALL(UF_MB_add_actions(actionTable));

	//register event for enable or disable ribbon buttons
	//UpdateButtonSensitivity();
	UF_registered_fn_p_t retFuncId;
	REGISTER_PART_EVENT_HANDLER(UF_change_work_part_reason, RMX_UI_on_workpart_changed, retFuncId);
	return true;
}