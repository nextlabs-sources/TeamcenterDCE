#pragma once

#include "CommonTypes.h"
#include <AcApDocWindow.h>

class CAcRMXApWinowMgrReactor : public AcApDocWindowManagerReactor
{
public:
	/// <summary>Fired when a document window is dragged out as floating dwg frame.</summary>
  /// <param name="docWindow">The document window being floated. </param>
	virtual void documentWindowFloated(AcApDocWindow* docWindow);
	/// <summary>Fired when a document window is docked from floating dwg frame back to the MDI child frame.</summary>
	/// <param name="docWindow">The document window being docked. </param>
	virtual void documentWindowDocked(AcApDocWindow* docWindow);

public:
	static CAcRMXApWinowMgrReactor* getInstance();
	static void             destroyInstance();

private:
	CAcRMXApWinowMgrReactor();
	virtual     ~CAcRMXApWinowMgrReactor();

	// outlawed functions
	RMX_DISABLE_COPY(CAcRMXApWinowMgrReactor)

private:
	static CAcRMXApWinowMgrReactor* m_pInstance;  // singleton instance
};

class CAcRMXWatermarkControl : public RMXControllerBase<CAcRMXWatermarkControl>
{
public:
	void Start();
	void Stop();

	bool SetViewOverlay(const CString& fileName);
	void UpdateViewOverlay(AcApDocument* pActivatedDoc);
	void RemoveViewOverlay();
	void RepositionOverlay();

	void ApplyAntiScreenCapture();
	void DisableUpdate(bool disable) { m_bUpdateDisabled = disable; }

private:
	bool m_bUpdateDisabled;

};

DEFINE_RMXCONTROLLER_GET(CAcRMXWatermarkControl, WatermarkController)