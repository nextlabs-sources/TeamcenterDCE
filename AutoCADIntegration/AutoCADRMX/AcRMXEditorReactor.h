//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 NextLabs, Inc.  All rights reserved.
//
//  Author: Joany Yang
//	Date:   Aug 2021
//////////////////////////////////////////////////////////////////////////////
//

#pragma once

class CAcRMXEditorReactor : public AcEditorReactor
{
public:
    ACRX_DECLARE_MEMBERS(CAcRMXEditorReactor);

    virtual void    beginSave(AcDbDatabase* pDwg, const TCHAR* szIntendedName);
    virtual void    saveComplete(AcDbDatabase* pDwg, const TCHAR* szActualName);

    // DWG Open Events.
    //
    virtual void endDwgOpen(const ACHAR* szFileName, AcDbDatabase* pDB);
    
    virtual void docCloseWillStart(AcDbDatabase* pDwg);

    virtual void commandWillStart(const ACHAR* szCmd);
    virtual void commandEnded(const ACHAR* szCmd);

    // Insert Events.
    //
    void beginInsert(AcDbDatabase* pTo, const ACHAR* pBlockName, AcDbDatabase* pFrom) override;
    void beginInsert(AcDbDatabase* pTo, const AcGeMatrix3d& xform, AcDbDatabase* pFrom) override;
    void abortInsert(AcDbDatabase* pTo) override;
    void endInsert(AcDbDatabase* pTo) override;

    // More XREF related Events
    // 
   /* void xrefSubcommandAttachItem(AcDbDatabase* pHost, int activity, const ACHAR* szPath) override;*/
    void xrefSubcommandDetachItem(AcDbDatabase* pHost, int activity, AcDbObjectId blockId) override;
    void xrefSubcommandReloadItem(AcDbDatabase* pHost, int activity, AcDbObjectId blockId) override;
    void xrefSubcommandUnloadItem(AcDbDatabase* pHost, int activity, AcDbObjectId blockId) override;

    virtual Acad::ErrorStatus xrefSubCommandStart(AcXrefSubCommand eSubCmd, 
                                                    const AcDbObjectIdArray& arrBtrIds, 
                                                    const ACHAR* const* szBtrNames, 
                                                    const ACHAR* const* szPaths);

    // Drawing area has moved or resized
    virtual void dwgViewResized(Adesk::LongPtr hwndDwgView);

public:
    static CAcRMXEditorReactor* getInstance();
    static void             destroyInstance();

private:
    CAcRMXEditorReactor();
    virtual     ~CAcRMXEditorReactor();

    // outlawed functions
    RMX_DISABLE_COPY(CAcRMXEditorReactor)

   void UpdateViewOverlay();
   bool ApplyUsageControlOnBeginInsert(AcDbDatabase* pFrom, AcDbDatabase* pTo);

private:
    static CAcRMXEditorReactor* m_pInstance;  // singleton instance
    std::wstring m_wsInsertingBlock;
};

