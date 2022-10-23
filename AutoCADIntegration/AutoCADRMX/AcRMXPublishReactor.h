//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Sep 2021
//////////////////////////////////////////////////////////////////////////////
//
#pragma once
class CAcRMXPublishReactor : public AcPublishReactor
{
public:
	ACRX_DECLARE_MEMBERS(CAcRMXPublishReactor);

    virtual void OnAboutToBeginPublishing(AcPublishBeginJobInfo* pInfo);
    virtual void OnEndPublish(AcPublishReactorInfo* pInfo);

public:
    static CAcRMXPublishReactor* getInstance();
    static void             destroyInstance();

private:
    CAcRMXPublishReactor();
    virtual     ~CAcRMXPublishReactor();

    // outlawed functions
    RMX_DISABLE_COPY(CAcRMXPublishReactor)

private:
    static CAcRMXPublishReactor* m_pInstance;  // singleton instance

};

