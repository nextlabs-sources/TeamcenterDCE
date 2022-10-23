#include <NXRMX/NXUsageControl.h>

#include <uf_mb.h>
#include <uf_assem.h>
#include <uf_part.h>
#include <NXRMX/nx_utils.h>
#include <NXRMX/PartContext.h>
#include <NXRMX/nxl_app.h>

#include <unordered_map>
#include <string>
#include <cctype>
#include <functional>

class PartNode
{
public:
	PartNode(tag_t part):_childNodes() {
		_tag = part;
	}
	inline tag_t tag() const { return _tag; }
	inline const std::vector<PartNode*> &childNodes() const { return _childNodes; }
	void addChild(PartNode * child) {
		if (std::find(_childNodes.begin(), _childNodes.end(), child) == _childNodes.end())
			_childNodes.push_back(child);
	}
	void debug(int level=0) {
		TRACELOG("%*s%u(Name='%s' nChild=%d)", level * 4, "", _tag, tag_to_name(_tag).c_str(), _childNodes.size());
		for (auto ichild = _childNodes.begin(); ichild != _childNodes.end(); ichild++) {
			(*ichild)->debug(level + 1);
		}
	}
private:
	tag_t _tag;
	std::vector<PartNode*> _childNodes;
};
class PartHierarchy
{
public:
	PartHierarchy(const std::vector<tag_t > & parts)
	{
		//build map and remove duplicated
		for (auto ipart = parts.begin(); ipart != parts.end(); ipart++) {
			if (_partNodes.find(*ipart) == _partNodes.end()) {
				_rootNodes.push_back(new PartNode(*ipart));//treate this node as root by default
				_partNodes.insert(std::make_pair(*ipart, _rootNodes.back()));
			}
		}
		//find direct parent
		for (auto inode = _partNodes.begin(); inode != _partNodes.end(); inode++) {
			//find the direct parent of this node
			tag_t *parentParts = NULL;
			int nparents = UF_ASSEM_where_is_part_used(inode->first, &parentParts);
			bool isRootNode = true;
			for (int i = 0; i < nparents; i++)
			{
				auto parentNode = _partNodes.find(parentParts[i]);
				if (parentNode != _partNodes.end()) {
					parentNode->second->addChild(inode->second);
					isRootNode = false;
				}
			}
			UF_free(parentParts);
			if (!isRootNode)
			{
				//remove this node from rootNodes
				auto node = std::find(_rootNodes.begin(), _rootNodes.end(), inode->second);
				if (node != _rootNodes.end()) {
					_rootNodes.erase(node);
				}
			}
		}
	}
	void clear() {
		for (auto inode = _partNodes.begin(); inode != _partNodes.end(); inode++) {
			delete inode->second;
		}
		_partNodes.clear();
		_rootNodes.clear();
	}
	void debug() {
		for (auto inode = _rootNodes.begin(); inode != _rootNodes.end(); inode++) {
			(*inode)->debug(0);
		}
	}
	inline const std::vector<PartNode*> &rootNodes(){ return _rootNodes; }
private:
	std::vector<PartNode*> _rootNodes;
	std::unordered_map<tag_t, PartNode*> _partNodes;
};
class PartRightsEvaluator {
public:
	PartRightsEvaluator(const std::vector<tag_t > & parts, const std::string &rightName):_allowed(true)
	{
		PartHierarchy partHierarchy(parts);
		partHierarchy.debug();
		auto rootNodes = partHierarchy.rootNodes();
		for (auto inode = rootNodes.begin(); inode != rootNodes.end(); inode++) {
			_allowed &= evalauteNode((*inode), rightName);
		}

		partHierarchy.clear();
	}
	inline bool isAllowed() const { return _allowed; }
	inline const std::vector<std::pair<std::string, std::string>> &denies() const { return _denies; }
private:
	bool evalauteNode(PartNode *node, const std::string &rightName)
	{
		//try find eval result from cache
		auto result = _cachedResult.find(node->tag());
		if (result != _cachedResult.end()) {
			//has been evaluated, use cached result
			return result->second;
		}
		bool allowed = true;
		auto context = PartContext::RetrieveContext(node->tag());
		if (context == nullptr)
		{
			NXDBG("Part-%u:ContextNotFound", node->tag());
		}
		else
		{
			NXDBG("Part-%u:" CONTEXT_FMT, node->tag(), CONTEXT_FMT_ARGS(context));
			if (context->is_protected())
			{
				auto requiredRight = rightName;
				g_rpmSession->CheckFileRights(context->part_file().wstr(), rightName.c_str(), allowed, true);
				if (allowed
					&& UF_PART_is_modified(context->part_tag())
					&& (rightName == RIGHT_SAVEAS //TODO:remove hardcode
						|| rightName == RIGHT_PRINT)) {
					requiredRight = RIGHT_EDIT;
					//when exporting a modified part, the edit right is also required
					g_rpmSession->CheckFileRights(context->part_file().wstr(), RIGHT_EDIT, allowed, true);
				}
				if (!allowed) {
					auto dispName = name_to_display(context->part_name());
					DBGLOG("Current user doesn't have %s on modified Part-'%s'", requiredRight.c_str(), dispName.c_str());
					_denies.push_back(std::make_pair(context->part_name(), requiredRight));
				}
			}
		}
		if (allowed)
		{
			//check children node only when this node is allowed
			for (auto ichild = node->childNodes().begin(); ichild != node->childNodes().end(); ichild++) {
				allowed &= evalauteNode(*ichild, rightName);
			}
		}
		//cache eval result
		_cachedResult.insert(std::make_pair(node->tag(), allowed));
		return allowed;
	}
	bool _allowed;
	std::unordered_map<tag_t, bool> _cachedResult;
	std::vector<std::pair<std::string, std::string>> _denies;

};
typedef std::vector<tag_t> (*pfGetContextParts)();
class ActionContext
{
public:
	ActionContext(const std::string &aName, const std::string &cName, const std::string &rName = "", pfGetContextParts getPartsCallback = nullptr) :
		actionName(aName), contextName(cName), rightName(rName), _getParts(getPartsCallback)
	{
		if (rightName.size() == 0)
		{
			rightName = Actions[actionName];
		}
		if (getPartsCallback == nullptr) {
			_getParts = Contexts[contextName];
		}
		std::string name = actionName;
		if (contextName.size() > 0)
		{
			name += "_" + contextName;
		}
		std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
		name = std::string(NXL_ACTION_PREFIX) + name;
		strcpy_s(fullName, sizeof(fullName), name.c_str());
	}
	std::vector<tag_t> GetParts() const {
		if (_getParts != nullptr)
			return _getParts();
		return std::vector<tag_t>();
	}
	char fullName[MAX_PATH];
	std::string actionName;
	std::string  rightName;
	std::string contextName;
	static std::vector<ActionContext> ActionContexts;
	static std::unordered_map<std::string, std::string> Actions;
	static std::unordered_map<std::string, pfGetContextParts> Contexts;
	pfGetContextParts _getParts;
};
std::unordered_map<std::string, std::string> ActionContext::Actions;
std::unordered_map<std::string, pfGetContextParts> ActionContext::Contexts;
std::vector<ActionContext> ActionContext::ActionContexts;

bool confirm_whole_file_download(const std::vector<tag_t> &parts)
{
	if (!nx_partial_download_enabled() && nx_isugmr())//only managed mode required confirmer
		return true;
	bool confirmYes = true;
	string_list_p messages = string_list_new();
	int nminimallyLoaded = 0;
	char partName[MAX_FSPEC_BUFSIZE] = "";
	//TODO:review this message
	string_list_add(messages, "Continue this operation may result following parts fully downloaded:");
	string_list_add(messages, "");
	for (auto i = parts.begin(); i != parts.end(); i++)
	{
		auto context = PartContext::FindByTag(*i);
		if (context == nullptr)
		{
			if (*i != NULLTAG
				&& NX_EVAL(UF_PART_ask_part_name(*i, partName)))
			{
				int loadStates = UF_PART_is_loaded(partName);
				NXDBG("%u-('%s'):loadStatus=%d", *i, partName, loadStates);
				if (loadStates != 1)
				{
					nminimallyLoaded++;
					string_list_add(messages, name_to_display(partName).c_str());
				}
			}
		}
	}
	string_list_add(messages, "");
	string_list_add(messages, "Do you want continue?");
	if (nminimallyLoaded > 0)
	{
#define RESPONSE_OK 1
#define RESPONSE_CANCEL 0
#define RESPONSE_OTHER -1
		int response = RESPONSE_OTHER;
		UF_UI_message_buttons_t buttons = { true, false, true, "Yes", NULL, "No", RESPONSE_OK, RESPONSE_OTHER, RESPONSE_CANCEL };
		NX_CALL(UF_UI_message_dialog(NX_RMX_DIALOG_TITLE, UF_UI_MESSAGE_QUESTION, messages->items, messages->count, false, &buttons, &response));
		NXDBG("User selected=%d", response);
		confirmYes = response == RESPONSE_OK;
	}
	string_list_free(messages);
	return confirmYes;
}
UF_MB_cb_status_t action_can_execute(
	UF_MB_widget_t             widget,
	UF_MB_data_t               client_data,
	UF_MB_activated_button_p_t call_button)
{
	NXDBG("=========================================");

	auto ret = UF_MB_CB_CONTINUE;
	if (NX_EVAL(UF_initialize()))
	{
		if (call_button != NULL)
		{
			NXDBG("Button Name=%s ID=%d AppId=%d MenuBarName='%s' NumOfAncestors=%d Type=%c", 
				call_button->name, call_button->id, call_button->app_id, call_button->menubar_name, call_button->num_ancestors, call_button->type);
		}
		if (client_data != NULL)
		{
			ActionContext *actionContext = (ActionContext*)client_data;
			auto parts = actionContext->GetParts();
			DBGLOG("Action(%s):RMRight=%s  ContextName=%s nParts=%d", actionContext->actionName.c_str(), actionContext->rightName.c_str(), actionContext->contextName.c_str(), parts.size());
			//NXDRMIntegration2007 only support the usage control for save and save as in unmanaged mode
			if(GetModuleHandle("NXDRMIntegration" STRINGIFY(NX_MAJOR_VER))!=NULL
				&&!nx_isugmr()//TBD:
				&& actionContext->actionName == ACTION_SAVE)//FOR SAVEAS, because only save as prt is support, NXRMX will still need check rights for other formats
			{
				INFOLOG("leave usage control to NXDRMIntegration");
			}
			else if (confirm_whole_file_download(parts)) {
				bool allAllowed = true;
				PartRightsEvaluator evaluator(parts, actionContext->rightName);
				if (!evaluator.isAllowed())
				{
					std::stringstream denyMessage;
					denyMessage << "You don't have specific permissions to " << actionContext->actionName << " following parts:"<<std::endl;
					for (auto imessage = evaluator.denies().begin(); imessage != evaluator.denies().end(); imessage++)
					{
						denyMessage << "\t" << imessage->first << "[" << imessage->second << "]"<<std::endl;
					}
					//TODO:notify
					nx_show_listing_window(denyMessage.str().c_str());
					ret = UF_MB_CB_CANCEL;
				}
			}
			else
			{
				ret = UF_MB_CB_CANCEL;
			}
		}		
		NX_CALL(UF_terminate());
	}

	NXDBG("Ends here.(ret=%d)", ret);
	return ret;

}

std::vector<tag_t> GetWorkPart()
{
	std::vector<tag_t> parts;
	parts.push_back(UF_ASSEM_ask_work_part()); 
	return parts;
}
std::vector<tag_t> GetAllModifiedParts()
{
	std::vector<tag_t> parts;
	auto allparts = part_list_all();
	for (auto p = allparts.begin(); p != allparts.end(); p++)
	{
		if (UF_PART_is_modified(*p))
		{
			NXDBG("PartIsModified:%d", *p);
			parts.push_back(*p);
		}
	}
	return parts; 
}
std::vector<tag_t> GetModifiedAndWorkPart()
{
	std::vector<tag_t> parts = GetAllModifiedParts();
	tag_t work = UF_ASSEM_ask_work_part();
	if (std::find(parts.begin(), parts.end(), work) == parts.end())
	{
		parts.push_back(work);
		NXDBG("WorkPart=%u", work);
	}
	return parts;
}

bool install_menu_script()
{
	/*
	TODO:the menu script file should be prevent from deleting
	TODO:the menu script file should be able to be updated automatically
	*/
	NxlPath mdir = NxlPath(SysUtils::GetModuleDir());
	NxlPath ucMenFile = mdir.AppendPath(MENU_USAGE_CONTROL);
	return ASSERT_FILE_EXISTS(ucMenFile);
}

void usage_control_on_part_opened(UF_callback_reason_e_t reason, const void *pTag, void *closure)
{
	if (NX_EVAL(UF_initialize()))
	{
		tag_t partTag = *((tag_t*)pTag);
		if (partTag != NULLTAG)
		{
			const PartContext *pContext = PartContext::FindByTag(partTag);
			if (pContext != nullptr
				&& pContext->is_protected())
			{
				bool allowed = false;
				g_rpmSession->CheckFileRights(pContext->part_file().wstr(), RIGHT_EDIT, allowed);
				if (!allowed)
				{
					std::stringstream message;
					message << "You don't have Edit right on Part-'" << name_to_display(pContext->part_name()) << "'" << std::endl;
					DBGLOG("%s", message.str().c_str());
					//TODO:notify
					nx_show_listing_window(message.str().c_str());
				}
			}
		}
		NX_EVAL(UF_terminate());
	}
}
/*----------------------------------------------------------------------------*
*  NXL_NXRMX_view_rights
*
*----------------------------------------------------------------------------*/
UF_MB_cb_status_t action_view_rights(
	UF_MB_widget_t             widget,
	UF_MB_data_t               client_data,
	UF_MB_activated_button_p_t call_button)
{
	NXDBG("=========================================");

	if (NX_EVAL(UF_initialize()))
	{
		auto parts = part_list_display_parts();
		std::stringstream message;
		for (auto p = parts.begin(); p != parts.end(); p++)
		{
			auto context = PartContext::FindByTag(*p);
			message << "[" << name_to_display(tag_to_name(*p).c_str()) << "]\t";
			if (context == nullptr)
			{
				message << "Unknown(may be minimally loaded)";
			}
			else if(context->is_protected())
			{
				//protected part
				message << ACTION_VIEW;
				for (auto r = ActionContext::Actions.begin(); r != ActionContext::Actions.end(); r++)
				{
					bool isAllowed;
					g_rpmSession->CheckFileRights(context->part_file().wstr(), r->second.c_str(), isAllowed);
					if (isAllowed)
					{
						message << ";" << r->first;
					}
				}
			}
			else
			{
				message << "Non-protected";
			}
			message << std::endl;
		}
		nx_show_listing_window(message.str().c_str());
		NX_CALL(UF_terminate());
	}

	NXDBG("Ends here.");
	return UF_MB_CB_CONTINUE;
}
bool install_usage_control()
{
	//add action mapping
	ActionContext::Actions.insert(std::make_pair(ACTION_SAVE, RIGHT_EDIT));
	ActionContext::Actions.insert(std::make_pair(ACTION_SAVEAS, RIGHT_SAVEAS));
	ActionContext::Actions.insert(std::make_pair(ACTION_EXPORT, RIGHT_SAVEAS));
	ActionContext::Actions.insert(std::make_pair(ACTION_PRINT, RIGHT_PRINT));

	//add context mapping
	ActionContext::Contexts.insert(std::pair<std::string, pfGetContextParts>(NXCONTEXT_ALL, part_list_all));
	ActionContext::Contexts.insert(std::pair<std::string, pfGetContextParts>(NXCONTEXT_ASSEMBLY, part_list_display_parts));
	ActionContext::Contexts.insert(std::pair<std::string, pfGetContextParts>(NXCONTEXT_WORK, GetWorkPart));
	ActionContext::Contexts.insert(std::pair<std::string, pfGetContextParts>(NXCONTEXT_ALLMODIFIED, GetAllModifiedParts));
	ActionContext::Contexts.insert(std::pair<std::string, pfGetContextParts>(NXCONTEXT_MODIFIED_AND_WORK, GetModifiedAndWorkPart));

	//build actions
	ActionContext::ActionContexts.push_back(ActionContext(ACTION_SAVE, NXCONTEXT_ALL));//save all
	ActionContext::ActionContexts.push_back(ActionContext(ACTION_SAVE, NXCONTEXT_ASSEMBLY));//save assembly
	ActionContext::ActionContexts.push_back(ActionContext(ACTION_SAVE, NXCONTEXT_WORK));//save work
	ActionContext::ActionContexts.push_back(ActionContext(ACTION_SAVE, NXCONTEXT_ALLMODIFIED));//save all modified
	ActionContext::ActionContexts.push_back(ActionContext(ACTION_SAVE, NXCONTEXT_MODIFIED_AND_WORK));//save modified and workpart

	ActionContext::ActionContexts.push_back(ActionContext(ACTION_SAVEAS, NXCONTEXT_ASSEMBLY));//save as assembly
	ActionContext::ActionContexts.push_back(ActionContext(ACTION_SAVEAS, NXCONTEXT_WORK));//save as work

	ActionContext::ActionContexts.push_back(ActionContext(ACTION_EXPORT, NXCONTEXT_ASSEMBLY));//export assembly
	ActionContext::ActionContexts.push_back(ActionContext(ACTION_EXPORT, NXCONTEXT_WORK));//export work

	ActionContext::ActionContexts.push_back(ActionContext(ACTION_PRINT, NXCONTEXT_ASSEMBLY));//print assembly
	ActionContext::ActionContexts.push_back(ActionContext(ACTION_PRINT, NXCONTEXT_WORK));//print work

	//install menu script
	install_menu_script();

	//register actions
	UF_MB_action_t actionTable[100];
	int nactions = 0;
	actionTable[nactions++] = { NXL_ACTION_VIEWRIGHTS, action_view_rights, NULL };
	actionTable[nactions++] = { APP_ACTION_FLCONFIRMER, action_force_load_confirmer, NULL };
	actionTable[nactions++] = { APP_ACTION_BLOCKER, action_blocker, NULL };
	for (auto i = ActionContext::ActionContexts.begin(); i != ActionContext::ActionContexts.end(); i++)
	{
		actionTable[nactions++] = { i->fullName, action_can_execute,  &(*i) };
		DBGLOG("[%d]%s:Action=%s Right=%s Context=%s cb=%p", nactions,i->fullName, i->actionName.c_str(), i->rightName.c_str(), i->contextName.c_str(), i->_getParts);
	}
	actionTable[nactions] = { NULL, NULL, NULL };
	NX_CALL(UF_MB_add_actions(actionTable));

	//check file rights on part loaded
	UF_registered_fn_p_t cbFuncId = NULL;
	//PartFileStream_SetReadOnly() will prevent user from saving the part
	//NX_CALL(UF_add_callback_function(UF_open_part_reason, usage_control_on_part_opened, NULL, &cbFuncId));

	return true;
}