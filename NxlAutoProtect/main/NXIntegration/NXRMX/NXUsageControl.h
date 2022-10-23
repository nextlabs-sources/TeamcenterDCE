#pragma once

#define MENU_USAGE_CONTROL L"nx_rmx_usage_control.men"

#define NXL_ACTION_PREFIX "NXL_NXRMX_"
#define NXL_ACTION_VIEWRIGHTS "NXL_NXRMX_view_rights"

/*NX Actions*/
#define ACTION_VIEW "View"
#define ACTION_SAVE "Save"
#define ACTION_SAVEAS "SaveAs"
#define ACTION_EXPORT "Export"
#define ACTION_PRINT "Print"

#define NXCONTEXT_ALL "all"
#define NXCONTEXT_ASSEMBLY "assembly"
#define NXCONTEXT_WORK "workpart"
#define NXCONTEXT_ALLMODIFIED "modified"
#define NXCONTEXT_MODIFIED_AND_WORK NXCONTEXT_ALLMODIFIED "_" NXCONTEXT_WORK

#define RIGHT_VIEW "RIGHT_VIEW"
#define RIGHT_EDIT "RIGHT_EDIT" 
//TODO:WORKAROUND:in skydrm server, the SaveAS right is defined as RIGHT_DOWNLOAD
#define RIGHT_SAVEAS "RIGHT_DOWNLOAD"
#define RIGHT_PRINT "RIGHT_PRINT"

bool install_usage_control();