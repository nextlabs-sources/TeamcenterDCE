#pragma once

namespace CatRMX {

	enum RightStatus
	{
		eRightGrant = 0,
		eRightDeny,
		eRightDenyEdit,
		eRightDenyFileNotExists,
		eRightDenyXref,
	};

	enum OpType
	{
		eUnsupportedOp,
		eSaveOp,
		eCopyOp,
		eExportOp,
		eProtectCommandOp
	};

	static std::set<std::wstring, ICaseKeyLess> SUPPORTED_FILETYPE_TABLE = {
		L"catpart", L"catdrawing", L"catproduct"
	};

	// Command name list  which will be handled by RMX
	// Note: The command name which has "RMX" prefix means it's fake one and sent by by RMX, 
	
	static const wchar_t* CMD_PRINT = L"Print";
	static const wchar_t* CMD_COPY = L"CATCCP_CopyCom";
	static const wchar_t* CMD_CUT = L"CATCCP_CutCom";

	//static const wchar_t* CMD_COPYFILESTODIRCMD = L"CopyFilesToDirCmd";
	static const wchar_t* CMD_PRODUCTTOPART = L"CATProductToPartStateCommand";
	//static const wchar_t* CMD_SENDFILESTOMAILCMD = L"SendFilesToMailCmd";
	static const wchar_t* CMD_CAPTURE = L"Capture";
	static const wchar_t* CMD_VIDEO = L"Video";
	static const wchar_t* CMD_PCS2CGR = L"CATSPPExportCGRCmd";
	static const wchar_t* CMD_PCS2PPR = L"CATSPPExportProductCmd";
	static const wchar_t* CMD_QUICKPRINT = L"ImmediatePrint";
	static const wchar_t* CMD_SAVECMD = L"SaveBox";
	static const wchar_t* CMD_SAVEASCMD = L"SavAsBox";
	static const wchar_t* CMD_CATSAVEALLAS = L"CATSaveAllAsCmd";
	static const wchar_t* CMD_CATSAVEALL = L"CATSaveAllCmd";

	// Drawing specific
	static const wchar_t* CMD_DRWPRINT = L"CATDrwPrint";
	static const wchar_t* CMD_DRWCUT = L"CATDrwCCP_CutCom";

	// Collabration
	static const wchar_t* CMD_COLLABSHARESNAPSHOT = L"CATInstantCollabSnapShotPreviewDlg";
	static const wchar_t* CMD_COLLABSHAREDOC = L"CATInstantCollabShareDocDlg";

	// RMX customized
	static const wchar_t* CMD_FILELOAD = L"RMXFileLoad";
	static const wchar_t* CMD_FILECLOSE = L"RMXFileClose";
	static const wchar_t* CMD_COPYONEFILETODIR = L"RMXCopyOneFileToDir";
	static const wchar_t* CMD_SCREENCAPTURE = L"RMXScreenCapture";
	static const wchar_t* CMD_SAVECHANGESTOFILE = L"RMXSaveChangesToFile";
	static const wchar_t* CMD_ACTIVATEWINDOW = L"RMXActivateWindow";
	static const wchar_t* CMD_FILESAVEASREADONLY = L"RMXSaveAsReadOnly";
	static const wchar_t* CMD_FRMCLOSEBOX = L"RMXFrmClosBox";
	static const wchar_t* CMD_FRMMDICHANGE = L"RMXFrmMDIChange";
	static const wchar_t* CMD_ADDCOMPFROMFILES = L"RMXAddCompFromFiles";
	static const wchar_t* CMD_UPDATENODECHILDREN = L"RMXUpdateNodeChildren";
	static const wchar_t* CMD_NEWFROM = L"RMXNewFrom";
	static const wchar_t* CMD_CAPTUREONACTIVATEWINDOW = L"RMXCaptureOnActivateWindow";
} // namespace CATRMX
