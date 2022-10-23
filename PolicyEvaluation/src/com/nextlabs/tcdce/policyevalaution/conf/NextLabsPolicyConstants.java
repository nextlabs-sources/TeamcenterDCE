package com.nextlabs.tcdce.policyevalaution.conf;

/**
 * Created on December 7, 2017
 * 
 * All sources, binaries and HTML pages (C) copyright 2017 by NextLabs Inc., San
 * Mateo CA, Ownership remains with NextLabs Inc, All rights reserved worldwide.
 */

public class NextLabsPolicyConstants {

	

	// policy_evaluation.properties
	public static final String NXL_POLICY_ACTION = "NXL_POLICY_ACTION";
	public final static String POLICY_EVALUATION_FILE = "policy_evaluation.properties";
	public final static String NXL_EVALUATION_ATTRIBUTES = "NXL_EVALUATION_ATTRIBUTES";
	public final static String PDP_HOST_NAME = "PDP_HOST_NAME";
	public final static String PDP_PORT_NUM = "PDP_PORT_NUM";
	public final static String NXL_PDP_DEFAULT_RESPONSE = "NXL_PDP_DEFAULT_RESPONSE";
	public final static String NXL_PDP_DEFAULT_MESSAGE = "NXL_PDP_DEFAULT_MESSAGE";
	public final static String DEFAULT_USER_NAME = "DEFAULT_USER_NAME";

	// Connection constants
	public static final int PC_CONN_TIMEOUT = 10000;

	// Response string key
	public static final String RESPONSE_KEY = "response";

	// Message string key
	public static final String RESPONSE_MESSAGE_KEY = "message";

	// Response string value - allow
	public static final String RESPONSE_ALLOW_VALUE = "allow";

	// Response string value - error
	public static final String RESPONSE_ERROR_KEY = "error";

	public static final String ATTR_KEY_SERVER = "server";
	public static final String ATTR_KEY_APPLICATION = "application";
	public static final String ATTR_KEY_NOCACHE = "ce::nocache";
	public static final String ATTR_VALUE_YES = "yes";
	public static final String ATTR_KEY_URL = "url";
	public static final String ATTR_VALUE_PORTAL_TYPE = "tc";

	// Policy evaluation
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
	// public static final String OPERATION_SOURCE =
	// "translator rights checker";

	// Constants for obligation
	public static final String CE_ATTR_OBLIGATION_COUNT = "CE_ATTR_OBLIGATION_COUNT";
	public static final String CE_ATTR_OBLIGATION_NAME = "CE_ATTR_OBLIGATION_NAME";

	// Obligation for RIGHT_EXECUTE_REMOVEPROTECTION
	public static final String OBLIGATION_REMOVEPROTECTION = "TCDRM_REMOVEPROTECTION";
}
