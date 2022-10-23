//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Aug 2021
//////////////////////////////////////////////////////////////////////////////
//

#pragma once

class CAcRMXApDocReactor : public AcApDocManagerReactor {

public:
	ACRX_DECLARE_MEMBERS(CAcRMXApDocReactor);

	// messages that are sent by notification
	virtual void	documentCreateStarted(AcApDocument* pDocCreating);

	virtual void	documentCreated(AcApDocument* pDocCreating);

	virtual void	documentCreateCanceled(AcApDocument* pDocCreateCancelled);

	virtual void	documentToBeDestroyed(AcApDocument* pDoc);

	virtual void    documentLockModeChanged(AcApDocument* pDoc,
												AcAp::DocLockMode eMyPreviousMode,
												AcAp::DocLockMode eMyCurrentMode,
												AcAp::DocLockMode eCurrentMode,
												const TCHAR* szGlobalCmdName);

	virtual void	documentActivated(AcApDocument* pActivatedDoc);

public:
	static CAcRMXApDocReactor* getInstance();
	static void             destroyInstance();

private:
	// outlawed functions
	// constructor/destructors
	CAcRMXApDocReactor();
	virtual         ~CAcRMXApDocReactor();
	RMX_DISABLE_COPY(CAcRMXApDocReactor)

private:
	// data members
	static CAcRMXApDocReactor* m_pInstance;  // singleton instance


};

