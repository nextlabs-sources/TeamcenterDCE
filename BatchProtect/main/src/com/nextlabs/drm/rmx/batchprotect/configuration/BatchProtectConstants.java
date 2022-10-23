package com.nextlabs.drm.rmx.batchprotect.configuration;

/**
 * Created on December 2, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

public class BatchProtectConstants {
	
	private BatchProtectConstants() {}
	
	// Dispatcher Service Framework: Name of the service provider
	public static final String DSF_PROVIDER = "NEXTLABS";
	
	// Dispatcher Service Framework: Name of the service
	public static final String DSF_SERVICE = "tonxlfile";
	
	// Dispatcher Service Framework: Priority used in processing the request
	public static final int DSF_PRIORITY = 2;
	
	//Argument Parameter
	public static final String ARG_NXL_ACTION = "nxlaction";
	public static final String ARG_NXL_REMARK = "nxlremark";
	
	// Batch protect supported actions
	public static final String ACT_PROTECT = "protect";
	public static final String ACT_UNPROTECT = "unprotect";
	public static final String ACT_DRYRUN = "dryrun";
	
	// File name for connection properties
	public static final String CONN_PROPERTIES_FILE = "connection.properties";
	
	// Properties for connection & behaviors
	public static final String TC_SOA_HOST = "TC_SOA_HOST";
	public static final String TC_USER_NAME = "TC_USER_NAME";
	public static final String TC_USER_PASSWORD = "TC_USER_PASSWORD";
	public static final String FORCE_UNCHECKOUT = "FORCE_UNCHECKOUT";
	
	// Default max number of return for query
	public static final int DEFAULT_MAX_NUM_TO_RETURN = 30;
	
	// Checkout Status Unknown
	public static final String DEFAULT_CHECKOUT_STATUS = " ";
	
	// Properties
	public static final String[] PROPS_MODELOBJS = new String[] {
		"object_name", "object_type", "checked_out", "active_seq"};
	
	// Property object name
	public static final String PROP_OBJECT_NAME = "object_name";
	
	// Property object type
	public static final String PROP_OBJECT_TYPE = "object_type";
	
	// Property checked out
	public static final String PROP_CHECKED_OUT = "checked_out";
	
	// Dataset Properties
	public static final String[] PROPS_DATASET = new String[] {
		"ref_list"};
	
	// ImanFile Properties
	public static final String[] PROPS_IMANFILE = new String[] {
		"byte_size"};
		
}
