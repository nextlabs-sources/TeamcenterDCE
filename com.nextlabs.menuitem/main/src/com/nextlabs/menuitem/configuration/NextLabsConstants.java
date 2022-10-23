package com.nextlabs.menuitem.configuration;

/**
 * Created on Aug 15, 2014
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2020 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

public interface NextLabsConstants {
		
	public static final String NEXTLABS_PROTECTED = "Nxl3_DRM_Protected";
	
	// supported commands
	public static final String CMD_PROTECT = "protect";
	public static final String CMD_UNPROTECT = "unprotect";
	public static final String LBL_CMD_UNPROTECT = "remove protection";
	
	// Message dialog box's title
	public static final String MSG_DIAG_TITLE = "NextLabs Rights Management";
	
	// Dispatcher Service Framework: Name of the service provider
	public static final String DSF_PROVIDER = "NEXTLABS";
	
	// Dispatcher Service Framework: Name of the service
	public static final String DSF_SERVICE = "tonxlfile";
	
	// Dispatcher Service Framework: Priority used in processing the request 
	public static final int DSF_PRIORITY = 2;
	
	
	// NextLabs Reference
	public static final String NEXTLABS_REFERENCE = "Nxl3_DRM_Protected_Reference";
	
	public static final String NEXTLABS_FILE_EXT = "nxl";
	
	
	// Secure View
	// TC preferences and environment variable for RMS Secure View
	public static final String PREF_HOSTNAME = "NXL_RMS_Hostname";
	public static final String PREF_DOMAIN = "NXL_RMS_Domain";
	public static final String PREF_POPUP = "NXL_RMS_Popup";
	public static final String PREF_TCSOA_HOSTNAME = "NXL_TCSOA_Hostname";
	public static final String NXL_RELATION_FILTER = "NXL_Relation_Filter";
	
	public static final String POPUP = "popup";
	public static final String TCSECUREVIEW_URL = "/RMS/RMSViewer/DisplayTCFile";
	public static final String TCLOADING_GIF = "/RMS/img/loading_48.gif";
	
	// Interval in miliseconds before remove the temp html file for post action
	public static final int INTERVAL_REM_HTMLPOST = 10000;
	
	// PDP connection details
	public final static String NXL_EVALUATION_ATTRIBUTES = "NXL_Evaluation_Attributes";
	public final static String NXL_PDPHOST_SERVERAPP = "NXL_PDPHOST_SERVERAPP"; // format: hostname/ip
	public final static String NXL_PDPPORT_SERVERAPP = "NXL_PDPPORT_SERVERAPP";
	public final static String NXL_PDP_HTTPS_SERVERAPP = "NXL_PDP_HTTPS_SERVERAPP";
	
	// CC connection details
	public final static String NXL_PDP_IGNOREHTTPS_SERVERAPP = "NXL_PDP_IGNOREHTTPS_SERVERAPP";
	public final static String NXL_OAUTH2HOST = "NXL_OAUTH2HOST";
	public final static String NXL_OAUTH2PORT = "NXL_OAUTH2PORT";
	public final static String NXL_OAUTH2_HTTPS = "NXL_OAUTH2_HTTPS";
	public final static String NXL_OAUTH2_CLIENTID = "NXL_OAUTH2_CLIENTID";
	public final static String NXL_OAUTH2_CLIENTSECRET = "NXL_OAUTH2_CLIENTSECRET";
	
	public final static String NXL_PDP_DEFAULT_ACTION = "NXL_PDP_DEFAULT_ACTION";
	public final static String NXL_PDP_DEFAULT_MESSAGE = "NXL_PDP_DEFAULT_MESSAGE";
	
	
	// Command buttons configuration
	public final static String NXL_PROTECT_ENABLE = "NXL_protect_cmd_enable";
	public final static String NXL_UNPROTECT_ENABLE = "NXL_unprotect_cmd_enable";
	
}
