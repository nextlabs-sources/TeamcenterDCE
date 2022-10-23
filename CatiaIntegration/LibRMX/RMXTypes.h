#pragma once

#define RMX_ERRORCODE_BASE                80000L
#define RMX_ERROR_NXLHELPER_NOTFOUND                (RMX_ERRORCODE_BASE + 1)
#define RMX_ERROR_INVALID_NXLFILE					(RMX_ERRORCODE_BASE + 2)

static const wchar_t* NXL_FILE_EXT = L".nxl";
static const wchar_t* NXL_FILE_EXT_WITHOUT_DOT = L"nxl";

#if defined(__cplusplus)
extern "C" {
#endif
	typedef enum {
		RMX_RIGHT_Nil = 0, /*Undefined rights, used to initialize*/
		RMX_RIGHT_VIEW = 1,
		RMX_RIGHT_EDIT = 2,
		RMX_RIGHT_PRINT = 4,
		RMX_RIGHT_CLIPBOARD = 8,
		RMX_RIGHT_SAVEAS = 0x10,
		RMX_RIGHT_DECRYPT = 0x20,
		RMX_RIGHT_SCREENCAPTURE = 0x40,
		RMX_RIGHT_SEND = 0x80,
		RMX_RIGHT_CLASSIFY = 0x100,
		RMX_RIGHT_SHARE = 0x200,
		RMX_RIGHT_DOWNLOAD = 0x400,
		RMX_RIGHT_WATERMARK = 0x40000000,
		RMX_RIGHT_NONE = 0x80000000 /*Always deny. No any right granted*/
	} RMXFileRight;

	typedef enum _RMX_SAFEDIRRELATION {
		RMX_NoneOfSafeDir = 0,  //No any relation of safe dir
		RMX_IsSafeDir,			//Is safe dir
		RMX_AncestorOfSafeDir,	//Ancestor of safe dir
		RMX_PosterityOfSafeDir	//Posterity of safe dir				
	} RMX_SAFEDIRRELATION;

	typedef enum _RMX_RPMFolderOption {
		RMX_RPMFOLDER_NORMAL = 1,
		RMX_RPMFOLDER_OVERWRITE = 4,
		RMX_RPMFOLDER_USER = 8,
		RMX_RPMFOLDER_EXT = 0x10,
		RMX_RPMFOLDER_API = 0x20
	} RMX_RPMFolderOption;

	//	* 0, not exit and not save
	//	* 1, not exit, but save
	//	* 2, exit and not save
	//	* 3 (and others), exit and save
	typedef enum _RMX_EDITSAVEMODE {
		RMX_EditSaveMode_None = 0,
		RMX_EditSaveMode_SaveOnly,
		RMX_EditSaveMode_ExitOnly,
		RMX_EditSaveMode_SaveAndExit
	} RMXEditSaveMode;

	typedef enum {
		RMX_OProtect = 1,
		RMX_OShare,
		RMX_ORemoveUser,
		RMX_OView,
		RMX_OPrint,
		RMX_ODownload,
		RMX_OEdit,
		RMX_ORevoke,
		RMX_ODecrypt,
		RMX_OCopyContent,
		RMX_OCaptureScreen,
		RMX_OClassify,
		RMX_OReshare,
		RMX_ODelete
	}RMXActivityLogOperation;

	typedef enum {
		RMX_RDenied = 0,
		RMX_RAllowed
	}RMXActivityLogResult;

	typedef struct _RMX_EVAL_ATTRIBUTE {
		const wchar_t* key;
		const wchar_t* value;
	} RMX_EVAL_ATTRIBUTE;

	typedef struct _RMX_EVAL_ATTRIBUTES {
		RMX_EVAL_ATTRIBUTE * attrs;
		size_t				 count;
	} RMX_EVAL_ATTRIBUTES;

	typedef struct _RMX_EVAL_FILES {
		const wchar_t** fileArr;
		size_t				 count;
	} RMX_EVAL_FILES;

	typedef struct _RMX_VIEW_OVERLAY_PARAMS {
		void* hTargetWnd;
		RMX_EVAL_FILES files;
		RMX_EVAL_ATTRIBUTES ctxAttrs;
		int displayOffset[4];
	} RMX_VIEW_OVERLAY_PARAMS;

#if defined(__cplusplus)
}
#endif

#define RMX_RIGHT_EXPORT RMX_RIGHT_DOWNLOAD
#define RMX_RIGHT_COPY RMX_RIGHT_DOWNLOAD
