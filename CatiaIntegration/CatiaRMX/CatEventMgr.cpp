#include "stdafx.h"
#include "CatEventMgr.h"

void CCatEventMgr::Start()
{
    LOG_INFO(L"Starting event manager...");
}

void CCatEventMgr::Stop()
{
    LOG_INFO(L"Stopping event manager...");
    m_listeners.clear();
}

void CCatEventMgr::AddListener(CCatEventLisener* listener, DWORD dwNotifies)
{
    m_listeners.push_back(make_pair(listener, dwNotifies));
}

void CCatEventMgr::RemoveListener(CCatEventLisener* listener, DWORD dwNotifies)
{
    auto iter = std::find_if(m_listeners.begin(), m_listeners.end(), [&](const std::pair<CCatEventLisener*, DWORD>& it)->bool {
        return ((listener == it.first) && (dwNotifies == it.second));
        });
    if (iter != m_listeners.end())
        m_listeners.erase(iter);
}

void CCatEventMgr::Notify(eCatNotifyType eNotifyType, LPCatNotify lParam)
{
  
    // TODO: Refactorying to have base callback and param type for all kinds of notify
    // to avoid more and more switch cases check?
    switch (eNotifyType)
    {
    case eCatNotify_BeforeCommand:
    {
        LPCatCommandNotify pParam = (LPCatCommandNotify)lParam;
        LOG_DEBUG_FMT(L"Sending notify(type: BeforeCommand, command=%s)", pParam->wsCmdName.c_str());
        for (const auto& lis : m_listeners)
        {
            if (HAS_BIT(lis.second, eNotifyType))
            {
                lis.first->OnBeforeCommand(pParam);
                if (pParam->bCancel)
                {
                    return;
                }
            }
        }

        break;
    } 
    case eCatNotify_AfterCommand:
    {
        LPCatCommandNotify pParam = (LPCatCommandNotify)lParam;
        LOG_DEBUG_FMT(L"Sending notify(type=AfterCommand, command:%s) ", pParam->wsCmdName.c_str());
        for (const auto& lis : m_listeners)
        {
            if (HAS_BIT(lis.second, eNotifyType))
            {
                lis.first->OnAfterCommand(pParam);
            }
        }

        break;
    }
    case eCatNotify_AfterSave:
    {
        LPCatSaveNotify pParam = (LPCatSaveNotify)lParam;
        LOG_DEBUG_FMT(L"Sending notify(type=AfterSave, file=%s)",  pParam->wsFileFullName.c_str());
        for (const auto& lis : m_listeners)
        {
            if (HAS_BIT(lis.second, eNotifyType))
            {
                lis.first->OnAfterSave(pParam);
            }
        }

        break;
    }
    case eCatNotify_FrmLayoutChanged:
    {
        LPCatFrmNotify pParam = (LPCatFrmNotify)lParam;
        LOG_DEBUG_FMT(L"Sending notify(type=FrmLayoutChanged, command=%s)", pParam->wsCmdName.c_str());
        for (const auto& lis : m_listeners)
        {
            if (HAS_BIT(lis.second, eNotifyType))
            {
                lis.first->OnFrmLayoutChanged(pParam);
            }
        }

        break;
    }
    case eCatNotify_FullScreenChanged:
    {
        int* pParam = (int*)lParam;
        LOG_DEBUG_FMT(L"Sending notify(type=FullScreenChanged, fullscreen=%d)", *pParam);
        for (const auto& lis : m_listeners)
        {
            if (HAS_BIT(lis.second, eNotifyType))
            {
                lis.first->OnFullScreenChanged(*pParam);
            }
        }

        break;
    }
    default:
        break;
    }
}
