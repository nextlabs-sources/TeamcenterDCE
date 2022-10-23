/*
 * Created on March 22, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2016 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.drm.internal.config;

public interface NextLabsConstants {
	
	public static final String TYPE_ITEM_REVISION = "ItemRevision";
	public static final String TYPE_DATASET = "Dataset";
	
	// key value argument for transaltion request
	public static final String KEY_NXLACTION = "nxlaction";
	
	// Dispatcher Service Framework: Name of the service provider
	public static final String DSF_PROVIDER = "NEXTLABS";
	//public static final String DSF_PROVIDER = "SIEMENS";
		
	// Dispatcher Service Framework: Name of the service
	public static final String DSF_SERVICE = "tonxlfile";
	//public static final String DSF_SERVICE = "simpgen";
		
	// Dispatcher Service Framework: Priority used in processing the request 
	public static final int DSF_PRIORITY = 2;
	
	// TC SOA REST API: Name of the SOA service 
	public static final String SOA_SERVICENAME = "Core-2008-06-DispatcherManagement";
	
	// TC SOA REST API: Name of the SOA service's operation
	public static final String SOA_OPERATIONNAME = "createDispatcherRequest";
		
	// 3.5 feature
	public final static String NXL_EVALUATION_ATTRIBUTES = "NXL_Evaluation_Attributes";
	public final static String NXL_PDPHOST_SERVERAPP = "NXL_PDPHOST_SERVERAPP"; // format: hostname/ip:port
	public final static String NXL_PDP_DEFAULT_ACTION = "NXL_PDP_DEFAULT_ACTION";
	public final static String NXL_PDP_DEFAULT_MESSAGE = "NXL_PDP_DEFAULT_MESSAGE";
	public final static int DEFAULT_PDP_PORT = 1099;
	
	public final static String NXL_RELATION_FILTER = "NXL_Relation_Filter";
	
}
