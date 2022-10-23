package com.nextlabs.drm.rmx.batchprotect.configuration;

/**
 * Created on December 2, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

public class NextLabsPolicyConstants {
	
	private NextLabsPolicyConstants() {}
		
	// Connection constants
	public static final int PC_CONN_TIMEOUT = 10000;
	public static final String TCRMX_PDP_ENGINE = "com.nextlabs.openaz.pdp.RestPDPEngine";
	public static final String TCRMX_PAYLOAD_JSON = "application/json";
	public static final String TCRMX_OAUTH2 = "OAUTH2";
	public static final String TCRMX_TOKEN_TYPE = "client_credentials";
	public static final String TCRMX_TOKEN_PATH = "/cas/token";
	public static final String TCRMX_PDP_ENGINEFAC = "com.nextlabs.openaz.pdp.PDPEngineFactoryImpl";
	public static final String TCRMX_MAPPER_KEY = "pep.mapper.classes";
	public static final String TCRMX_MAPPER_VAL = "com.nextlabs.openaz.pepapi.RecipientMapper,com.nextlabs.openaz.pepapi.DiscretionaryPoliciesMapper,com.nextlabs.openaz.pepapi.HostMapper,com.nextlabs.openaz.pepapi.ApplicationMapper";
	
	public static final String RIGHT_EXECUTE_REMOVEPROTECTION = "RIGHT_EXECUTE_REMOVEPROTECTION";
	
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
	
	// Response string key
	public static final String RESPONSE_KEY = "response";
	
	// Message string key
	public static final String RESPONSE_MESSAGE_KEY = "message";
	
	// Response string value - allow
	public static final String RESPONSE_ALLOW_VALUE = "allow";
	
	// Response string value - error
	public static final String RESPONSE_ERROR_KEY = "error";
	
	public static final String ATTR_KEY_SERVER = "server";
	public static final String ATTR_KEY_HOST = "host";
	public static final String ATTR_KEY_APPLICATION = "application";
	public static final String ATTR_KEY_NOCACHE = "ce::nocache";
	public static final String ATTR_VALUE_YES = "yes";
	public static final String ATTR_KEY_URL = "url";
	public static final String ATTR_VALUE_PORTAL_TYPE = "tc";
	
	public static final String DEFAULT_ACTION = "allow";
	public static final String DEFAULT_MESSAGE = "PDP connection timeout";
	
	// Policy evaluation
	public static final String CE_APPLICATION = "TC_BatchProtect";
	public static final String CE_USER = "TC_Proxy";
	public static final String CE_NA = "N/A";
	
	public static final String PC_OWNING_USER = "owning_user";
	
	// Constants for resource URL
	public static final String ATTRIBUTE_REV_TYPE = "rev-type";
	public static final String ATTRIBUTE_REV_NAME = "rev-name";
	public static final String ATTRIBUTE_REV_UID = "rev-uid";
	public static final String ATTRIBUTE_DS_TYPE = "ds-type";
	public static final String ATTRIBUTE_DS_NAME = "ds-name";
	public static final String ATTRIBUTE_DS_UID = "ds-uid";
	public static final String ATTRIBUTE_OPERATION_SOURCE = "operation-source";
	public static final String OPERATION_SOURCE = "batch protect";
	
	// Constants for obligation
	public static final String CE_ATTR_OBLIGATION_COUNT = "CE_ATTR_OBLIGATION_COUNT";
	public static final String CE_ATTR_OBLIGATION_NAME = "CE_ATTR_OBLIGATION_NAME";
	
	// Obligation for RIGHT_EXECUTE_REMOVEPROTECTION
	public static final String OBLIGATION_REMOVEPROTECTION = "TCDRM_REMOVEPROTECTION";
	
}
