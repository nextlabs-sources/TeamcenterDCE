#include <NXRMX/PartContext.h>
#include <uf_part.h>
#include <hook/hook_defs.h>
#include <NXRMX/nx_utils.h>
#include <set>

class WindowInfo
{
public:
	WindowInfo(HWND hWnd=nullptr) :_hnd(hWnd),_visible(false),_childWindow(NULL)
	{
		std::stringstream ss;
		ss << static_cast<void*>(_hnd);
		if (_hnd != nullptr)
		{
			int len = GetWindowTextLength(hWnd) + 1;
			char buffer[MAX_PATH];
			GetWindowText(hWnd, buffer, MAX_PATH);
			_windowText = buffer;
			GetClassNameA(hWnd, buffer, MAX_PATH);
			_className = buffer;
			_visible = IsWindowVisible(hWnd);
			ss << "(Visible = " << _visible << " Text = '" << _windowText << "' ClassName = '" << _className << "')";
		}
		_str = ss.str();
	}
	inline bool isvalid() const { return _hnd != nullptr; }
	inline HWND hnd() const { return _hnd; }
	inline const char *text() const { return _windowText.c_str(); }
	inline const char *clsName() const { return _className.c_str(); }
	inline bool visible() const { return _visible; }
	inline const char *str() const { return _str.c_str(); }
	bool operator==(const HWND &hnd) const
	{
		return _hnd == hnd;
	}
	bool operator!=(const HWND &hnd) const
	{
		return _hnd != hnd;
	}
	bool operator==(const WindowInfo &w) const
	{
		return _hnd == w._hnd;
	}
	bool operator!=(const WindowInfo &w) const
	{
		return _hnd != w._hnd;
	}
	operator bool() const {
		return _hnd != nullptr;
	}
	WindowInfo GetRootWindow() const
	{
		if (_hnd == nullptr) return *this;
		return GetAncestor(_hnd, GA_ROOT);
	}
	WindowInfo FindFirstChildWindow(bool visibleOnly = false, const char *className = nullptr)
	{
		_childWindow = nullptr;
		return FindNextChildWindow(visibleOnly, className);
	}
	WindowInfo FindNextChildWindow(bool visibleOnly = false, const char *className = nullptr)
	{
		WindowInfo childWindow(_childWindow);
		do
		{
			if (childWindow) {
				//next child
				childWindow = GetWindow(childWindow.hnd(), GW_HWNDNEXT);
			}
			else
			{
				//first child
				childWindow = GetWindow(_hnd, GW_CHILD);
			}
			if ((className == nullptr || strcmp(className, childWindow.clsName()) == 0)
				&& (!visibleOnly || childWindow.visible()))
			{
				break;
			}
		} while (childWindow);
		_childWindow = childWindow.hnd();
		return childWindow;
	}
private:
	HWND _hnd;
	HWND _childWindow;
	std::string _str;
	std::string _className;
	std::string _windowText;
	bool _visible;
};

struct EnumWindowParams
{
	DWORD in_dwProcessId;
	HWND in_hWnd;
	WindowInfo out_window;
};
BOOL CALLBACK FindNextVisibleWindowInInputProcessCB(HWND hWnd, LPARAM lParam)
{
	auto pParams = (EnumWindowParams*)(lParam);
	if (pParams->in_hWnd != nullptr)
	{
		//
		if (hWnd == pParams->in_hWnd)
		{
			//found, reset
			pParams->in_hWnd = nullptr;
		}
		//goto next handle
		return TRUE;
	}
	DWORD dwProcessId;
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	if (dwProcessId != pParams->in_dwProcessId)
		return TRUE;
	WindowInfo wndInfo(hWnd);
	if (wndInfo.visible())
	{
		pParams->out_window = wndInfo;
		//DBGLOG("FoundWindow-%s", wndInfo.str());
		return FALSE;
	}
	//DBGLOG("Window-%s", wndInfo.str());
	return TRUE;
}
class NXMDIWindow
{
public:
	NXMDIWindow(HWND topWindow = nullptr, HWND mdiWindow = nullptr, tag_t part=NULLTAG) :_topWindow(topWindow), _mdiWindow(mdiWindow), _displayedPart(part)
	{
		update_str();
	};
	inline const char *str() const
	{
		return _str.c_str();
	}
	//TODO:workaround:RPM SDK only support overlay on top window
	inline HWND overlay() const { return _topWindow; }
	inline HWND mdi() const { return _mdiWindow; }
	inline HWND top() const { return _topWindow; }
	inline void set_top(HWND top) { _topWindow = top; update_str(); }
	inline tag_t displayedPart() const { return _displayedPart; }
	inline void setDisplayedPart(tag_t p) { _displayedPart = p; _openingParts.clear(); update_str(); }
	inline void addOpeningParts(tag_t part) {
		auto i = _openingParts.insert(part);
		if(i.second)
			update_str();
	}
	inline std::vector<tag_t> getRootParts() const {
		std::vector<tag_t> rootParts;
		rootParts.push_back(_displayedPart);
		for (auto i = _openingParts.begin(); i != _openingParts.end(); i++) {
			rootParts.push_back(*i);
		}
		return rootParts;
	}
	inline bool isvalid() const { return overlay() != nullptr; }
	inline void assign(const NXMDIWindow &window)
	{
		_topWindow = window._topWindow;
		_mdiWindow = window._mdiWindow;
		_displayedPart = window._displayedPart;
		_openingParts = window._openingParts;
		update_str();
	}
	bool operator==(const HWND &hnd) const
	{
		return _mdiWindow == hnd;
	}
	bool operator!=(const HWND &hnd) const
	{
		return _mdiWindow != hnd;
	}
	bool operator==(const NXMDIWindow &w) const
	{
		return _mdiWindow == w._mdiWindow
			&& _topWindow == w._topWindow
			&& _displayedPart == w._displayedPart;
	}
	bool operator!=(const NXMDIWindow &w) const
	{
		return _mdiWindow != w._mdiWindow
			|| _topWindow != w._topWindow
			|| _displayedPart != w._displayedPart;
	}
	operator bool()const
	{
		return isvalid();
	}
private:
	inline void update_str()
	{
		std::stringstream ss;
		ss << static_cast<void*>(_mdiWindow) << "(TopWindow=" << static_cast<void*>(_topWindow);
		if (_displayedPart != NULLTAG)
		{
			ss << " DisplayedPart=" << _displayedPart << "['" << tag_to_name(_displayedPart) << "']";
		}
		if (!_openingParts.empty())
			ss << " nOpeningParts=" << _openingParts.size();
		ss << ")";
		_str = ss.str();
	}
	std::string _str;
	tag_t _displayedPart;
	std::set<tag_t> _openingParts;
	HWND _topWindow;
	HWND _mdiWindow;
};

class NXOverlay
{
public:
	static NXOverlay *s_nxOverlay;
	virtual NXMDIWindow& GetActiveMDIWindow()=0;
	virtual void OnPartUpdated(tag_t modifiedPart) {
		std::set<HWND> updatingOverlays;
		DBGLOG("Updating overlay for windows displaying part-%u", modifiedPart);
		for (auto imdi = _mdiWindows.begin(); imdi != _mdiWindows.end(); imdi++)
		{
			if (modifiedPart == imdi->displayedPart())
			{
				imdi->setDisplayedPart(modifiedPart);//reset opening parts
				DBGLOG("MdiWindow:%s", imdi->str());
			}
			else
			{
				int occs = part_ask_occs_of(modifiedPart, imdi->displayedPart());
				DBGLOG("MdiWindow:%s: has %d occs of part-%u", imdi->str(), occs, modifiedPart);
				if (occs <= 0)
				{
					continue;
				}
			}
			updatingOverlays.insert(imdi->overlay());
		}
		if (updatingOverlays.size())
		{
			auto activeWindow = GetActiveMDIWindow();
			auto mdi = WindowInfo(activeWindow.mdi());
			for (auto ioverlay = updatingOverlays.begin(); ioverlay != updatingOverlays.end(); ioverlay++)
			{
				RefreshTopWindow(*ioverlay, mdi);
			}
		}
		DBGLOG("Updated %d windows", updatingOverlays.size());
	}
	virtual void OnPartAdded(tag_t addedpart)
	{
		tag_t currentDisplayedPart = UF_PART_ask_display_part();
		if (currentDisplayedPart == NULLTAG)
			return;
		int occs = part_ask_occs_of(addedpart, currentDisplayedPart);
		NXDBG("Opening part-%u has %d occs of display part-%u", addedpart, occs, currentDisplayedPart);
		if (occs <= 0)
		{
			//opening  part is not part of current displaypart, add it as opening part
			NXMDIWindow& activeWindow = GetActiveMDIWindow();
			activeWindow.addOpeningParts(addedpart);
			RefreshTopWindow(activeWindow.overlay(), activeWindow.mdi());
		}
		else {
			auto partname = tag_to_name(addedpart);
			int loadStates = UF_PART_is_loaded(partname.c_str());
			NXDBG("%u('%s'):loadStatus=%d", addedpart, partname.c_str(), loadStates);
			if (loadStates == 2) {
				//partial load
				//sometime the part open event occurred after work part changed
				RefreshActiveWindow();
			}
		}
	}
	NXOverlay():_cbFuncPartOpened(nullptr)
	{
		UF_registered_fn_p_t cbFuncPartSaved;
		_rmxAttrs = kvl_new(3);
		kvl_add(_rmxAttrs, "rmxname", "NXRMX");
		kvl_add(_rmxAttrs, "nx.pim", nx_isugmr() ? "yes" : "no");
		NX_CALL(UF_add_callback_function(UF_change_work_part_reason, NXOverlay::OnNXWorkPartChanged, NULL, &_cbFuncWorkPartChanged));
		NX_CALL(UF_add_callback_function(UF_save_part_reason, NXOverlay::OnNXPartUpdated, NULL, &cbFuncPartSaved));
		NX_CALL(UF_add_callback_function(UF_open_part_reason, NXOverlay::OnNXPartOpened, NULL, &_cbFuncPartOpened));
	}
	static void OnNXPartOpened(UF_callback_reason_e_t reason, const void* pTag, void* closure)
	{
		if (pTag == NULL)
			return;
		if (NX_EVAL(UF_initialize()))
		{
			tag_t openedPart = *((tag_t*)pTag);
			if (openedPart != NULLTAG)
			{
				s_nxOverlay->OnPartAdded(openedPart);
			}
			NX_EVAL(UF_terminate());
		}

	}
	static void OnNXPartUpdated(UF_callback_reason_e_t reason, const void* pTag, void* closure)
	{
		if (pTag == NULL)
			return;
		if (NX_EVAL(UF_initialize()))
		{
			tag_t modifiedPart = *((tag_t*)pTag);
			if (modifiedPart != NULLTAG)
			{
				s_nxOverlay->OnPartUpdated(modifiedPart);
			}
			NX_EVAL(UF_terminate());
		}
	}
	static void OnNXWorkPartChanged(UF_callback_reason_e_t reason, const void *pTag, void *closure)
	{
		if (NX_EVAL(UF_initialize()))
		{
			static NXMDIWindow prevDisplayWindow;
			tag_t currentDisplayedPart = UF_PART_ask_display_part();
			if (prevDisplayWindow.displayedPart() != currentDisplayedPart)
			{
				NXMDIWindow &activeWindow  = s_nxOverlay->GetActiveMDIWindow();
				activeWindow.setDisplayedPart(currentDisplayedPart);//reset opening parts
				if (prevDisplayWindow.mdi() == activeWindow.mdi())
				{
					DBGLOG("Displayed Part changed from %u to %u", prevDisplayWindow.displayedPart(), currentDisplayedPart);
				}
				else
				{
					DBGLOG("Displayed Window switched from %s to %s", prevDisplayWindow.str(), activeWindow.str());
				}
				NXOverlay::s_nxOverlay->RefreshTopWindow(activeWindow.overlay(), activeWindow.mdi());
				prevDisplayWindow = activeWindow;
			}
			else {
				NXMDIWindow& activeWindow = s_nxOverlay->GetActiveMDIWindow();
				int nopening = activeWindow.getRootParts().size() - 1;
				if (nopening > 0) {
					NXDBG("nopening=%d", nopening);
					//contain opening part, reset
					activeWindow.setDisplayedPart(currentDisplayedPart);//reset opening parts
					NXOverlay::s_nxOverlay->RefreshTopWindow(activeWindow.overlay(), activeWindow.mdi());
				}
			}
			NX_EVAL(UF_terminate());
		}
	}
	static bool IsMdiDisplayed(const NXMDIWindow& mdi) {
		//WindowInfo firstChild = GetTopWindow(mdi.hnd());
		//DBGLOG("%s:FirstChild-%s", mdi.str(), firstChild.str());
		//return mdi.FindNextChildWindow().isvalid();
		WindowInfo mdiwindow(mdi.mdi());
		WindowInfo childWindow = mdiwindow.FindFirstChildWindow();
		DBGLOG("MdiWindow=%s:FirstChild=%s", mdiwindow.str(), childWindow.str());
		return mdiwindow.visible() || childWindow.isvalid();
	}
	virtual void RefreshActiveWindow()
	{
		NXMDIWindow &activeWindow = GetActiveMDIWindow();
		RefreshTopWindow(activeWindow.overlay(), activeWindow.mdi());
	}
	void OnParentWindowChanged(HWND hndWindow, HWND hndNewParent) {
		auto imdiWindow = std::find(_mdiWindows.begin(), _mdiWindows.end(), hndWindow);
		if (imdiWindow != _mdiWindows.end())
		{
			DBGLOG("MDIWindowParentChanged-%s", imdiWindow->str());
			auto mdiWindow = WindowInfo(hndWindow);
			auto newTopWindow = mdiWindow.GetRootWindow();
			DBGLOG("==>NewTopWindow-%s", newTopWindow.str());
			auto prevOverlay = imdiWindow->overlay();
			imdiWindow->set_top(newTopWindow.hnd());
			RefreshTopWindow(newTopWindow, imdiWindow->mdi());
			//found top window
			if (prevOverlay != newTopWindow.hnd())
			{
				RefreshTopWindow(prevOverlay, WindowInfo());
			}
		}
	}
	/*
	NX 12/1872 window layout
	one mdi child window displays one assembly
	one renering window belongs to one top window
	MDI child window can be moved from one top window to another top window
	one top window may contain multiple MDI child window
	one top window may display multiple MDI child window at same time
	*/
	void RefreshTopWindow(const WindowInfo& overlayWindow, const WindowInfo &mdiWindow)
	{
		DBGLOG("Refreshing TopWindow-%s(activeMdiWindow-%s)...", overlayWindow.str(), mdiWindow.str());
		std::set<tag_t> visibleParts;
		const PartContext* rootContext = nullptr;
		for (auto i = _mdiWindows.begin(); i != _mdiWindows.end(); i++)
		{
			if (i->overlay() == overlayWindow.hnd())
			{
				if (mdiWindow == i->mdi()//NOTE:sometime the moving window is not visible at this time, and has no children
					|| IsMdiDisplayed(*i))
				{
					DBGLOG("ActiveMdiWindow-%s", i->str());
					auto rootParts = i->getRootParts();
					for (auto iroot = rootParts.begin(); iroot != rootParts.end(); iroot++)
					{
						auto insert = visibleParts.insert(*iroot);
						if (insert.second)
						{
							//newroot
							auto parts = assembly_list_parts(*iroot);
							for (auto ipart = parts.begin(); ipart != parts.end(); ipart++)
							{
								visibleParts.insert(*ipart);
							}
						}
					}
					if (mdiWindow == i->mdi()) {
						rootContext = PartContext::FindByTag(i->displayedPart());
					}
				}
				else
				{
					DBGLOG("InactiveMdiWindow-%s", i->str());
				}
			}
			else
			{
				DBGLOG("MdiWindow-%s", i->str());
			}
		}
		//update rmx attribute for the part informaton
		//use the root part of the active mdi window as the context part file
		//this part may not be protected
		std::string partName, fileName, filePath;
		if (rootContext != nullptr) {
			partName = name_to_display(rootContext->part_name());
			filePath = rootContext->part_file().string();
			fileName = PathFindFileNameA(filePath.c_str());
		}
		kvl_setOrAdd(_rmxAttrs, "partname", partName.c_str());
		kvl_setOrAdd(_rmxAttrs, "filename", fileName.c_str());
		kvl_setOrAdd(_rmxAttrs, "filepath", filePath.c_str());
		OnWindowContextPartsUpdated(overlayWindow, visibleParts);
	}
	void OnWindowContextPartsUpdated(const WindowInfo &overlayWindow, const std::set<tag_t> &contextParts)
	{
		string_list_p ctxFiles = string_list_new();
		bool containsMinimalLoadedParts = false;
		for (auto i = contextParts.begin(); i != contextParts.end(); i++)
		{
			//auto context = PartContext::RetrieveContext(*i);
			auto context = nx_isugmr() ? PartContext::FindByTag(*i) : PartContext::RetrieveContext(*i);
			if (context == nullptr)
			{
				DBGLOG("Part-%u is minimally loaded", *i);
				containsMinimalLoadedParts = true;
			}
			else if(context->is_protected())
			{
				DBGLOG("[%d]%u-[%s]:File-'%s' tag='%s'", ctxFiles->count, *i, context->part_name(), context->part_file().str(), context->tags().str());
				string_list_add(ctxFiles, context->part_file().str());
			}
		}
		DBGLOG("OverlayWindow-%s nParts=%d nProtectedParts=%u", overlayWindow.str(), contextParts.size(), ctxFiles->count);
		g_rpmSession->UpdateSecureOverlay(_rmxAttrs, ctxFiles, overlayWindow.hnd());
		if (containsMinimalLoadedParts) {
			g_rpmSession->SetScreenCapture(overlayWindow.hnd(), false);
		}
		else
		{
			g_rpmSession->UpdateScreenCapture(overlayWindow.hnd(), ctxFiles);
		}
		string_list_free(ctxFiles);
	}
protected:
	std::vector<NXMDIWindow> _mdiWindows;
	UF_registered_fn_p_t _cbFuncWorkPartChanged;
	UF_registered_fn_p_t _cbFuncPartOpened;
	kvl_p _rmxAttrs;
};

class NX11Overlay :public NXOverlay
{
public:
	NX11Overlay() :NXOverlay() {}
	NXMDIWindow &GetActiveMDIWindow() override
	{
		//NX 11 has only one mdi window that is never changed
		//the window hierrachy is as following
		//Application Top Window(Text='NX 11 - Modeling')
		//		MdiWindowContainer(ClassName=MDIClient)
		//			MdiWindow for displayed assembly(ClassName=MDIClient)
		if (!_mdiWindows.size())
		{
			NXMDIWindow window;
			EnumWindowParams paramTopWindow = { GetCurrentProcessId() };
			DBGLOG("Searching Top Window of Current Process-%lu", paramTopWindow.in_dwProcessId);
			while (!EnumWindows(FindNextVisibleWindowInInputProcessCB, (LPARAM)&paramTopWindow)
				&& paramTopWindow.out_window.isvalid())
			{
				//found a top level window of current process
				auto childWindow = paramTopWindow.out_window.FindNextChildWindow(true, "MDIClient");
				DBGLOG("==>MDIContainer-%s:ChildWindow-%s", paramTopWindow.out_window.str(), childWindow.str());
				// When the top level window is found for the given model, try to find out
				// the child window for model display
				if (childWindow.isvalid())
				{
					//TODO:Overlay SDK only support top window
					window = NXMDIWindow(paramTopWindow.out_window.hnd(), childWindow.hnd());
					break;
				}
				//go to next top window
				paramTopWindow.in_hWnd = paramTopWindow.out_window.hnd();
			}
			_mdiWindows.push_back(window);
		}
		auto displaypart = UF_PART_ask_display_part();
		if(displaypart!= _mdiWindows.front().displayedPart())
			_mdiWindows.front().setDisplayedPart(displaypart);
		return _mdiWindows.front();
	}
};
typedef
HWND(WINAPI *pfSetParent)(
	__in HWND hWndChild,
	__in_opt HWND hWndNewParent);

static pfSetParent SetParent_original;
static pfSetParent SetParent_next;
extern HWND WINAPI SetParent_hooked(
	__in HWND hWndChild,
	__in_opt HWND hWndNewParent);
class NX12Overlay :public NXOverlay
{
public:
	NX12Overlay() :NXOverlay()
	{
		//mornitoring window parent change event
		HOOK(SetParent);
	}
	NXMDIWindow& GetActiveMDIWindow() override {
		auto activeWindow = SearchActiveMDIWindow();
		//update window layout information
		auto ichild = std::find(_mdiWindows.begin(), _mdiWindows.end(), activeWindow.mdi());
		if (ichild != _mdiWindows.end())
		{
			if (activeWindow != *ichild)
			{
				DBGLOG("WindowUpdated:from %s to %s", ichild->str(), activeWindow.str());
				ichild->assign(activeWindow);
			}
			else
			{
				DBGLOG("ActiveWindow-%s", ichild->str());
			}
			return *ichild;
		}
		else
		{
			//new MDI child window
			DBGLOG("NewMDIWindow-%s", activeWindow.str());
			_mdiWindows.push_back(activeWindow);
			return _mdiWindows.back();
		}
	}
	static NXMDIWindow SearchActiveMDIWindow()
	{
		//the window hierrachy is as following
		//Application Top Window(Text='NX 12 - Modeling'):there may be multiple top windows
		//		MdiWindowContainer(ClassName=NX_SURFACE_WND)
		//			MdiWindow 1 for assembly 1(ClassName=NX_SURFACE_WND IsVisible=true if visible)
		//				ChildWindow(if current active window)
		//			MdiWindow 2 for assembly 2(ClassName=NX_SURFACE_WND IsVisible=true if visible)
		//			...
		//Floating windows(ClassName=NX_SURFACE_WND): there may be multiple float windows
		//		MdiWindowContainer(ClassName=NX_SURFACE_WND)
		//			MdiWindow 1 for assembly 3(ClassName=NX_SURFACE_WND IsVisible=true if active)
		//			MdiWindow 2 for assembly 4(ClassName=NX_SURFACE_WND IsVisible=true if active)
		HWND topWindow = nullptr;
		EnumWindowParams paramsTopwindow = { GetCurrentProcessId() };
		DBGLOG("Searching Top Window of Current Process-%lu", paramsTopwindow.in_dwProcessId);
		while (!EnumWindows(FindNextVisibleWindowInInputProcessCB, (LPARAM)&paramsTopwindow)
			&& paramsTopwindow.out_window.isvalid())
		{
			//found a top level window of current process
			// When the top level window is found for the given model, try to find out
			// the child window for model display
			WindowInfo mdiContainer;
			DBGLOG("TopWindow-%s:Searching MDIContainer Window(ClassName='NX_SURFACE_WND' Visible=1)...", paramsTopwindow.out_window.str());
			while ((mdiContainer = paramsTopwindow.out_window.FindNextChildWindow(true, "NX_SURFACE_WND")).isvalid())
			{
				WindowInfo mdiWindow;
				DBGLOG("==>MDIContainer-%s:Searching MDI window Window(ClassName='NX_SURFACE_WND' Visible=1)...", mdiContainer.str());
				while ((mdiWindow = mdiContainer.FindNextChildWindow(true, "NX_SURFACE_WND")).isvalid())
				{
					NXMDIWindow mdi(paramsTopwindow.out_window.hnd(), mdiWindow.hnd());
					//there may be multiple visible MDI window existing in same container
					if (IsMdiDisplayed(mdi))
					{
						DBGLOG("====>>MDIWindow-%s is active", mdiWindow.str());
						//this MDI window is active
						mdi.setDisplayedPart(UF_PART_ask_display_part());
						return mdi;
					}
					//this MDI window is inactive, move to next
					DBGLOG("====>>MDIWindow-%s is inactive", mdiWindow.str());
				}
			}
			//go to next top window
			paramsTopwindow.in_hWnd = paramsTopwindow.out_window.hnd();
		}
		auto mdi = NXMDIWindow(topWindow);
		mdi.setDisplayedPart(UF_PART_ask_display_part());
		return mdi;
	}
};
HWND WINAPI SetParent_hooked(
	__in HWND hWndChild,
	__in_opt HWND hWndNewParent)
{
	HWND ret;
	//CALL_START("SetParent(hWndChild=%p, hWndNewParent=%p)", hWndChild, hWndNewParent);
	ret = SetParent_next(hWndChild, hWndNewParent);
	NXOverlay::s_nxOverlay->OnParentWindowChanged(hWndChild, hWndNewParent);
	//CALL_END("SetParent(hWndChild=%p, hWndNewParent=%p) returns %p", hWndChild, hWndNewParent);
	return ret;
}
NXOverlay *NXOverlay::s_nxOverlay;

void overlay_force_refresh()
{
	NXDBG("Refreshing secure overlay..");
	NXOverlay::s_nxOverlay->RefreshActiveWindow();
}
bool install_secure_overlay()
{
	switch (nx_get_major_version())
	{
	case 9:
	case 10:
	case 11:
		NXOverlay::s_nxOverlay = new NX11Overlay();
		break;
	case 12:
	case 1872:
	default:
		NXOverlay::s_nxOverlay = new NX12Overlay();
		break;
	}
	//check file rights on part loaded
	return true;
}