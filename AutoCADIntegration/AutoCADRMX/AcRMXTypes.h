#pragma once

#define ACRMX_MESSAGEDLG_LABEL L"NextLabs RMX"

namespace AcRMX {

	enum AccessRightStatus
	{
		eRightGrant = 0,
		eRightDeny,
		eRightDenyEdit,
		eRightDenyFileNotExists,
		eRightDenyXref,
		eRightDenyUnderlay
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
		L"dwg", L"dwf", L"dwt", L"dwfx", L"dws", L"dxf"
	};
} // namespace AcRMX
