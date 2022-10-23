#pragma once

#include <set>
#include <string>
#include "..\common\CommonTypes.h"

typedef std::wstring TModelId;

#define OTKX_COMMAND_SAVE			"ProCmdModelSave"

#define OTKX_COMMAND_SAVEAS			"ProCmdModelSaveAs"
#define OTKX_COMMAND_BACKUP			"ProCmdModelBackup"
#define OTKX_COMMAND_MIRRORPART		"ProCmdMirrorPart"
#define OTKX_COMMAND_MIRRORASM		"ProCmdMirrorAssem"
#define OTKX_COMMAND_SAVEASCSA		"ProCmdSaveAsCSA"
#define OTKX_COMMAND_SAVEASMODULE	"ProCmdSaveAsModule"
#define OTKX_COMMAND_TC_SAVEAS		"txd_iManCIAs"
#define OTKX_COMMAND_APPMESH		"ProCmdUtilAppMech"

#define OTKX_COMMAND_QUICKEXPORT	"ProCmdQuickExport"
#define OTKX_COMMAND_EXPORTPREVIEW	"ProCmdExportPreview"
#define OTKX_COMMAND_PUBLISHEXPORT	"ProCmdDwgPubExport"
#define OTKX_COMMAND_DWGPUBPREVIEW	"ProCmdDwgPubPreviewOpen"

#define OTKX_COMMAND_PRINT			"ProCmdModelPrint"
#define OTKX_COMMAND_PRINT2D		"ProCmdModelPrint2D"
#define OTKX_COMMAND_PRINT3D		"ProCmdModelPrint3D"
#define OTKX_COMMAND_QUICKPRINT		"ProCmdQuickPrint"
#define OTKX_COMMAND_QUICKDRAWING	"ProCmdQuickDrawing"
#define OTKX_COMMAND_3DPRINT		"ProCmdModel3dPrint"
#define OTKX_COMMAND_3DPRINTORDER	"ProCmd3dPrintOrder"
#define OTKX_COMMAND_EMAILATTACH	"ProCmdModelEmailAttach"
#define OTKX_COMMAND_WINCLOSE		"ProCmdWinClose"
#define OTKX_COMMAND_3DPRINTFREEMIUM "ProCmd3dPrintFreemium"
#define OTKX_COMMAND_3DPRINTSAVEAS   "ProCmd3dPrintSaveAs"

#define OTKX_COMMAND_JTEXPORTCURRENT "EAI Export"
#define OTKX_COMMAND_JTUSEBATCH		 "EAI Batch"

#define OTKX_COMMAND_TC_ABOUT		"txd_iManAbout"
#define OTKX_COMMAND_TC_SAVE		"txd_iManCI"
#define OTKX_COMMAND_TC_SAVEALL		"txd_iManCIAll"
#define OTKX_COMMAND_TC_CACHEMANAGER	"txd_cacheManager"
#define OTKX_COMMAND_TC_CANCELCO	"txd_iManCOCancel"
#define OTKX_COMMAND_TC_OPEN		"txd_iManCO"

//#define OTKX_PARAM_NXL_ORIGIN		"NXL_ORIGIN"

enum ProtectSourceAction { SOURCEACTION_SAVE, SOURCEACTION_COPY, SOURCEACTION_EXPORT, SOURCEACTION_PROTECT};

#define PROTECT_WATCHER_SAVE                    0x0001
#define PROTECT_WATCHER_COPY					0x0002
#define PROTECT_WATCHER_EXPORT					0x0004
#define PROTECT_WATCHER_EXPORT_JTBATCH			0x0008 
#define PROTECT_WATCHER_EXPORT_JTCURRENT		0x0010
#define PROTECT_WATCHER_SAVE_TC                 0x0020
#define PROTECT_WATCHER_EXPORT_PLAYBACK_MOTIONENVELOPE                0x0040
#define PROTECT_WATCHER_BACKUP					0x0080
#define PROTECT_WATCHER_SHRINKWRAP				0x0100
#define PROTECT_WATCHER_TC_CACHEMANAGER			0x0200
#define PROTECT_WATCHER_TC						0x0400
#define PROTECT_WATCHER_SENDMAIL				0x0800
#define PROTECT_WATCHER_EXPORT_DWGPUBPREVIEW	0x1000

// /*?/ comment means the file type is listed in hardware notes doc but not in save as dialog
static const std::set<std::wstring, ICaseKeyLess> EXPORT_FILETYPE_TABLE = {
	/*Image formats*/L"eps", L"jpg", L"pdf", L"pic", L"shd", L"tif",
	/*2D formats*/L"cgm", L"dwg", L"dxf", L"igs", L"she", L"stp", L"tsh", L"png",
	/*3D formats*/
	L"asc",
	L"iv",
	L"g",
	L"jt",
	L"neu",
	L"gbf",
	L"x_t",
	L"ed", L"edz", L"pvs", L"pvz", L"plt", L"ol",
	L"slp",
	L"sldprt", L"sldasm",
	L"stl",
	L"u3d",
	L"prt",
	L"vda",
	L"wrl",
	/*ECAD formats*/
	L"mdb"/*?*/, L"mdc"/*?*/, L"mdf"/*?*/,
	L"edn"/*?*/, L"edp"/*?*/,
	L"idx",
	L"emn", L"emp", L"eda",
	L"nwf"/*?*/,
	L"evs"/*?*/,
	L"unv",
	L"sat",
	L"obj", L"mtl"/*?*/,
	L"facet",
	L"ntr",
	L"amf",
	L"3mf"
};

static std::set<std::wstring, ICaseKeyLess> DXFFILE_BLACKLIST = { 
	L"pro_medusa", L"autocad1", L"temp_dwg", L"pro_sth" 
};
