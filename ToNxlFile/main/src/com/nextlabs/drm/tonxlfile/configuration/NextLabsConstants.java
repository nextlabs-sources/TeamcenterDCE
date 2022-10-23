package com.nextlabs.drm.tonxlfile.configuration;

/**
 * Created on Aug 4, 2015
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2019 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

public abstract class NextLabsConstants {
	
	public static final String NXL_FILE_EXT = "nxl";
	public static final String NXL_NAMED_REFERENCE = "Nxl3_DRM_Protected_Reference";
	
	public static final String CHARSET = "UTF-8";
	
	// Argument Parameter
	public static final String ARG_NXL_ACTION = "nxlaction";
	public static final String ARG_NXL_REMARK = "nxlremark"; // 3.5 feature: stores client's request context, i.e workflow handler
	
	// Actions
	public static final String ACT_PROTECT = "protect";
	public static final String ACT_UPDATETAG = "updateTag";
	public static final String ACT_REMOVEPROTECTION = "unprotect";
	public static final String ACT_REFRESHDUID = "refreshduid";
	
	// Properties
	public static final String[] PROPS_DATASET_TASKPREP = new String[] {
		"ref_list", "ref_names", "last_mod_date", "last_mod_user", "checked_out_change_id", "checked_out_user"};
	public static final String[] PROPS_DATASET = new String[] {
		"revision_number", "ref_list", "ref_names", "ref_types"};
	public static final String[] PROPS_DATASET_REV = new String[] {
		"revision_number", "revisions_prop"};
	public static final String[] PROPS_NAMED_REFERENCES = new String[] {
		"file_ext", "file_name", "original_file_name"};
	
	public static final String PROP_CHECKED_OUT = "checked_out";
	public static final String PROP_CHECKED_OUT_CHANGE_ID = "checked_out_change_id";
	public static final String NXL_LAST_MOD_USER = "nxl3_last_mod_user";
	public static final String NXL_LAST_MOD_DATE = "nxl3_last_mod_date";
	public static final String NXL_IS_NXL_PROTECTED = "nxl3_is_nxl_protected";
	public static final String FORMAT_USED = "format_used";
	
	// TaskPrep
	public static final String PRIMARYOBJECTS = "primaryObjects";
	public static final String SECONDARYOBJECTS = "secondaryObjects";
	public static final String ARGUMENTKEYS = "argumentKeys";
	public static final String ARGUMENTDATA = "argumentData";
	
	public static final String INPUT_PATH = "INPUTPATH";
	public static final String INPUT_FILENAME = "INPUTFILENAME";
	public static final String OUTPUT_PATH = "OUTPUTPATH";
	public static final String NXL_ACTION = "NXLACTION";
	public static final String INPUT_ARGS_FILENAME = "INPUTARGSFILENAME";
	public static final String TAG_PREFIX = "TAG";
	public static final String SRC_LIST_FILENAME = "filelist.txt";
	public static final String ARGS_FILENAME = "argsfile.txt";
	public static final String TRANSIENT_FILENAME = "transientdata.txt";
	// fix bug 38138
	// special character that can't be used in filename (Windows platform)
	public static final String SRC_LIST_DELIMITER = ":";
	
	public static final int TRANSLATOR_NOOFTAGS_CONFIGURED = 10;
	
	// TC Preferences (need to be same with the preferences defined in the setup folder
	public static final String NXL_TRANSFERRED_ATTRIBUTES = "NXL_Transferred_Attributes";
	public static final String NXL_REMOVE_SOURCEFILE = "NXL_Remove_Sourcefile";
	
	// PDP connection details
	public static final String NXL_EVALUATION_ATTRIBUTES = "NXL_Evaluation_Attributes";
	public static final String NXL_PDPHOST_SERVERAPP = "NXL_PDPHOST_SERVERAPP"; // format: hostname/ip
	public static final String NXL_PDPPORT_SERVERAPP = "NXL_PDPPORT_SERVERAPP";
	public static final String NXL_PDP_HTTPS_SERVERAPP = "NXL_PDP_HTTPS_SERVERAPP";
	
	// CC connection details
	public static final String NXL_PDP_IGNOREHTTPS_SERVERAPP = "NXL_PDP_IGNOREHTTPS_SERVERAPP";
	public static final String NXL_OAUTH2HOST = "NXL_OAUTH2HOST";
	public static final String NXL_OAUTH2PORT = "NXL_OAUTH2PORT";
	public static final String NXL_OAUTH2_HTTPS = "NXL_OAUTH2_HTTPS";
	public static final String NXL_OAUTH2_CLIENTID = "NXL_OAUTH2_CLIENTID";
	public static final String NXL_OAUTH2_CLIENTSECRET = "NXL_OAUTH2_CLIENTSECRET";
	
	public static final String NXL_PDP_DEFAULT_ACTION = "NXL_PDP_DEFAULT_ACTION";
	public static final String NXL_PDP_DEFAULT_MESSAGE = "NXL_PDP_DEFAULT_MESSAGE";
	
	// 4.5 feature
	public static final String NXL_INSTALLED_FEATURES = "NXL_Installed_Features";
	
	// 5.2 feature
	// Testing feature for bug 60268
	public static final String NXL_PURGE_REV_SEQ = "NXL_PURGE_REV_SEQ";
	
	// 3.0 feature
	public static final String NXL_NR_BLACKLIST = "NXL_NR_Blacklist";
	
	// 3.0 feature: To cater for metadata sync cases for Document Revision
	/*
	 *  Document Revision checkin, the dataset will be checkin as well, 
	 *  but sequence may not correspond with batch job execution order
	 */
	public static final String NXL_DS_CHKOUT_RETRYCOUNT = "NXL_DS_Chkout_RetryCount";
	public static final String NXL_DS_CHKOUT_WAITTIME = "NXL_DS_Chkout_WaitTime";
	
	public static final String TONXLFILEDATA = "tonxlfile.bin";
	public static final String DSTYPE_ALL = "AllTypes";
	

	
	// Checkout Status Unknown
	public static final String DEFAULT_CHECKOUT_STATUS = " ";
	
	// Property checked out
	public static final String NXL_CHECKOUT_MSG = "NextLabs EDRM in progress";
	public static final String NXL_CHANGEID = "NXL-PROTECT";
	
	// Dispatcher request query
	public static final String[] DSF_PROVIDER = new String[]{ "NEXTLABS" };
	public static final String[] DSF_SERVICE = new String[]{ "tonxlfile" };
	// interested in the following states, excepts COMPLETE, TERMINAL
	public static final String[] DSF_STATES = new String[] {"PREPARING", 
		"SCHEDULED", "TRANSLATING", "LOADING"}; 
	
	// NXL Installed Features
	public static final String SUPPLIER_COLLABORATION_FOUNDATION = "supplier_collaboration_foundation";
	public static final String BRIEFCASE_BROWSER = "briefcase_browser";
	
	public static final String LINE_SEPARATOR = "line.separator";
	
}
