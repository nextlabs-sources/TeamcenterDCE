#include "hook_defs.h"
#include "hook_utils.h"
#include "hook_log.h"
#include "TCXMLParser.h"
#include <tie/tie.h>
#include <gms/gms.h>
#include <sa/tcfile.h>
#include <Shlwapi.h>
#define file_exist(f) (f != NULL && GetFileAttributes(f) != INVALID_FILE_ATTRIBUTES)
/*
	Creating a Child Process with Redirected Input and Output
	Refer to:https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx
*/
BOOL process_run(char *cmd)
{
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;
	BOOL ret = FALSE;
	// Set up members of the PROCESS_INFORMATION structure.
	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
	// Set up members of the STARTUPINFO structure.
	// This structure specifies the STDIN and STDOUT handles for redirection.
	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO);
	//siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;
	siStartInfo.wShowWindow = SW_HIDE;

	DBGLOG("CreateProcess:'%s'", cmd);
	// Create the child process.
	bSuccess = CreateProcess(NULL,
			cmd,     // command line
			NULL,          // process security attributes
			NULL,          // primary thread security attributes
			TRUE,          // handles are inherited
			0,             // creation flags
			NULL,          // use parent's environment
			NULL,          // use parent's current directory
			&siStartInfo,  // STARTUPINFO pointer
			&piProcInfo);  // receives PROCESS_INFORMATION

	// If an error occurs, exit the application.
	if ( ! bSuccess )
	{
		DBGLOG(TEXT("CreateProcess Failed(%lu)"), GetLastError());
	}
	else
	{
		while( TRUE ) {
			BOOL status = WaitForSingleObject(piProcInfo.hProcess, INFINITE);
			switch(status)
			{
				case WAIT_TIMEOUT:
					//WAIT TIMEOUT, MAYBE THE pipe buffer is full, it's waiting for a readfile
					DBGLOG("WaitForSingleObject(%lu) TimeOut", piProcInfo.dwProcessId);
					break;
				case WAIT_FAILED:
					DBGLOG("WaitForSingleObject(%lu) Failed:%lu", piProcInfo.dwProcessId, GetLastError());
					break;
				case WAIT_OBJECT_0:
					break;
				case WAIT_ABANDONED:
				default:
					DBGLOG("WaitForSingleObject(%lu) Status=%lu", piProcInfo.dwProcessId, status);
					break;
			}
			DWORD retCode;
			BOOL r = GetExitCodeProcess(piProcInfo.hProcess, &retCode);
			if(r == 0)
			{
				DBGLOG("GetExitCodeProcess(%lu) Failed:%lu", piProcInfo.dwProcessId, GetLastError());
				break;
			}
			else if(r != STILL_ACTIVE)
			{
				DBGLOG("GetExitCodeProcess(%lu) returns %d(ExitCode=%lu)", piProcInfo.dwProcessId, r, retCode);
				break;
			}
		}
STOP_WAIT:
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example.
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
		ret = TRUE;
	}
	return ret;
}
void run_modifier(const char *pname, const char *file, int nargs, char **args)
{
	if(pname != NULL && file != NULL)
	{
		char cmd[0x1000];
		HMODULE module;
		//initialize current module path
		if((module = GetModuleHandle("NxlHook.dll")) != NULL)
		{
			int len  = GetModuleFileName(module, cmd, sizeof(cmd));
			if(len > 0)
			{
				//DBGLOG("GetModuleFileName=%s", dirBuf);
				PathRemoveFileSpec(cmd);
				len = strlen(cmd);
				len += sprintf(cmd+len, "\\%s", pname);
				if(file_exist(cmd))
				{
					int iarg = 0;
					len += sprintf(cmd+len, " \"%s\"", file);
					for(iarg = 0; iarg < nargs; iarg++)
					{
						DBGLOG("arg-%d:'%s'", iarg, args[iarg]);
						len += sprintf(cmd+len, " \"%s\"", args[iarg]);
					}
					DBGLOG("Executing(%d)....'%s'", len, cmd);
					process_run(cmd);
				}
				else
				{
					DBGLOG("FileNotFound-'%s'", cmd);
				}
			}
			else
			{
				DBGLOG("Failed to GetModuleFileName!(ErrorCode:%#X)", GetLastError());
			}
		}
	}
}
/*
	IMF_import_file
*/
#define IMF_import_file_FULLNAME "IMF_import_file"
//TODO:
#define IMF_import_file_ORDINAL 0
typedef int (*pfIMF_import_file)(const char *  osFileName,
	const char *  newFilename,
	int  fileType,
	tag_t *  newFileTag,
	IMF_file_t *  fileDescriptor
);

static pfIMF_import_file IMF_import_file_original;
static pfIMF_import_file IMF_import_file_next;
static int IMF_import_file_hooked(
	const char *  osFileName,
	const char *  newFilename,
	int  fileType,
	tag_t *  newFileTag,
	IMF_file_t *  fileDescriptor
   )
{
	int ret;
	CALL_START("IMF_import_file(osFileName='%s', newFileName='%s' fileType=%d)", osFileName, newFilename, fileType);

	if(StringEndsWithA(osFileName, ".bcz", FALSE))
	{
		run_modifier("bczModifier.exe", osFileName, 0, NULL);
	}


	CALL_NEXT(ret = IMF_import_file_next(osFileName, newFilename, fileType, newFileTag, fileDescriptor));

	CALL_START("IMF_import_file(osFileName='%s', newFileName='%s' fileType=%d, *newFileTag=%u *fileDescriptor=%p) return %d", osFileName, newFilename, fileType, *newFileTag, *fileDescriptor, ret);

	return ret;
}
/*
	GMS_export_objects_to_offline_package
*/
#define GMS_export_objects_to_offline_package_FULLNAME "GMS_export_objects_to_offline_package"
//TODO:
#define GMS_export_objects_to_offline_package_ORDINAL 0
typedef int (*pfGMS_export_objects_to_offline_package)(
    const char *    reason,                 /* <I> */
    int             n_objects,              /* <I> */
    tag_t *         object_tags,            /* <I> */
    int             n_sites,                /* <I> */
    tag_t *         sites,                  /* <I> */
    tag_t           option_set_tag,         /* <I> */
    int             n_options,              /* <I> */
    const char **   option_names,           /* <I> */
    const char **   option_values,          /* <I> */
    int             n_session_options,      /* <I> */
    const char **   session_option_names,   /* <I> */
    const char **   session_option_values,  /* <I> */
    tag_t *         briefcase_dataset_tag,  /* <O> */
    char **         offline_file_name,      /* <OF> */
    int  *          n_export_log_files,     /* <O> */
    char ***        export_log_files        /* <OF> */
);
static pfGMS_export_objects_to_offline_package GMS_export_objects_to_offline_package_original;
static pfGMS_export_objects_to_offline_package GMS_export_objects_to_offline_package_next;
static int GMS_export_objects_to_offline_package_hooked(
    const char *    reason,                 /* <I> */
    int             n_objects,              /* <I> */
    tag_t *         object_tags,            /* <I> */
    int             n_sites,                /* <I> */
    tag_t *         sites,                  /* <I> */
    tag_t           option_set_tag,         /* <I> */
    int             n_options,              /* <I> */
    const char **   option_names,           /* <I> */
    const char **   option_values,          /* <I> */
    int             n_session_options,      /* <I> */
    const char **   session_option_names,   /* <I> */
    const char **   session_option_values,  /* <I> */
    tag_t *         briefcase_dataset_tag,  /* <O> */
    char **         offline_file_name,      /* <OF> */
    int  *          n_export_log_files,     /* <O> */
    char ***        export_log_files        /* <OF> */)
{
	init_nxl_scf_hook_log();
	int ret = 0;
	int i;
	CALL_START("GMS_export_objects_to_offline_package(reason='%s', option_set_tag=%u)", reason);
	DBGLOG("n_session_options='%d'", n_session_options);
	for(i=0; i<n_session_options; i++)
	{
		DBGLOG("==>sessionOptions[%d/%d]:'%s'='%s'", i+1, n_session_options, session_option_names[i], session_option_values[i]);
	}
	HOOK_API("libsa.dll", IMF_import_file);

	CALL_NEXT(ret = GMS_export_objects_to_offline_package_next(reason,
		n_objects, object_tags,
		n_sites, sites, option_set_tag,
		n_options, option_names, option_values,
		n_session_options, session_option_names, session_option_values,
		briefcase_dataset_tag, offline_file_name, n_export_log_files, export_log_files));

	UNHOOK(IMF_import_file);
	CALL_END("GMS_export_objects_to_offline_package() returns '%d'", ret);
	DBGLOG("==>briefcase_dataset_tag=%u", *briefcase_dataset_tag);
	DBGLOG("==>offline_file_name='%s'", *offline_file_name);
	DBGLOG("==>n_export_log_files=%d", *n_export_log_files);
	for(i=0; i<*n_export_log_files; i++)
	{
		DBGLOG("==>export_log_files[%d/%d]=%s", i+1, *n_export_log_files, (*export_log_files)[i]);
	}
	return ret;
}

/*
	TIE_export_objects
*/
#define TIE_export_objects_FULLNAME "TIE_export_objects"
#define TIE_export_objects_ORDINAL 2569
typedef int (*pfTIE_export_objects)(
    const char *tcGSMessageId,      /**< (I) Unique id for transaction */
    int n_objects,                  /**< (I) Number of objects to export */
    tag_t* objects,                 /**< (I) n_objects Objects to export */
    int n_siteTags,                 /**< (I) Number of sites to export to */
    tag_t* targetSiteTags,          /**< (I) n_siteTags Tags of target sites */
    logical synchronize,            /**< (I) Synchronize objects */
    TIE_transfer_formula_t opts,    /**< (I) Structure of options */
    TIE_output_data_t* output_data  /**< (I) Output data structure*/
    );
static pfTIE_export_objects TIE_export_objects_original;
static pfTIE_export_objects TIE_export_objects_next;
static int TIE_export_objects_hooked(
    const char *tcGSMessageId,      /**< (I) Unique id for transaction */
    int n_objects,                  /**< (I) Number of objects to export */
    tag_t* objects,                 /**< (I) n_objects Objects to export */
    int n_siteTags,                 /**< (I) Number of sites to export to */
    tag_t* targetSiteTags,          /**< (I) n_siteTags Tags of target sites */
    logical synchronize,            /**< (I) Synchronize objects */
    TIE_transfer_formula_t opts,    /**< (I) Structure of options */
    TIE_output_data_t* output_data  /**< (I) Output data structure*/
    )
{
	int ret = 0;
	int i;
	CALL_START("TIE_export_objects(tcGSMessageId='%s' synchronize=%d )", tcGSMessageId, synchronize);

	DBGLOG("opts.optionSetName='%s'", opts.optionSetName);
	DBGLOG("opts.n_sessOptions='%d'", opts.n_sessOptions);
	bool bcz_export = true;
	for(i=0; i<opts.n_sessOptions; i++)
	{
		DBGLOG("opts.sessionOptions[%d/%d]:'%s'='%s'", i+1, opts.n_sessOptions, opts.sessionOptions[i].name, opts.sessionOptions[i].value);
		if (strcmp(opts.sessionOptions[i].name, "opt_nonbcz_export") == 0) {
			bcz_export = false;
		}
	}
	DBGLOG("opts.transferModeName='%s'", opts.transferModeName);
	DBGLOG("opts.reason='%s'", opts.reason);
	CALL_NEXT(ret = TIE_export_objects_next(tcGSMessageId, n_objects, objects, n_siteTags, targetSiteTags, synchronize, opts, output_data));
	if (strcmp(opts.optionSetName,"SRMIntegrationOptionSet") == 0) {
		DBGLOG("Ignore TcXML export with SRMIntegrationOptionSet");
	} else {
		DBGLOG("output_data->xmlFilePath='%s'", output_data->xmlFilePath);
		DBGLOG("output_data->importexport_log_fmsticket='%s'", output_data->importexport_log_fmsticket);
		DBGLOG("output_data->failedobjects_fmsticket='%s'", output_data->failedobjects_fmsticket);
		DBGLOG("output_data->nRootObjs='%d'", output_data->nRootObjs);
		if (bcz_export) {
			char *args[2] = {};
			int tier = ITK__ask_managed_mode()?4:2;
			args[0] = "-transientvolume";
			DBGLOG("Tier=%d", tier);
			IMF_get_transient_volume_root_dir(tier, &(args[1]));
			DBGLOG("IMF_get_transient_volume_root_dir='%s'", args[1]);
			//run_modifier("XmlModifier.exe", output_data->xmlFilePath, sizeof(args)/sizeof(char*), args);
			TCXMLParser tcXMLParser;
			tcXMLParser.parseTCXML_Export(std::string(output_data->xmlFilePath), std::string(args[1]));
			MEM_free(args[1]);
		} else {
			DBGLOG("DDE Package not send inside a bcz file. Skip TcXML file modify step.");
		}
	}
	CALL_END("TIE_export_objects() returns '%d'", ret);
	return ret;
}


BOOL tcserver_install()
{
	HOOK_API("libtie.dll", TIE_export_objects);
	HOOK_API("libgms.dll", GMS_export_objects_to_offline_package);
	return TRUE;
}
void tcserver_uninstall()
{
}
