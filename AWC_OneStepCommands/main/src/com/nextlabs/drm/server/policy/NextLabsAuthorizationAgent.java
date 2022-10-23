package com.nextlabs.drm.server.policy;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.HashMap;
import java.util.List;
import java.util.Map.Entry;

import com.nextlabs.destiny.sdk.CEApplication;
import com.nextlabs.destiny.sdk.CEAttributes;
import com.nextlabs.destiny.sdk.CEEnforcement;
import com.nextlabs.destiny.sdk.CEResource;
import com.nextlabs.destiny.sdk.CESdk;
import com.nextlabs.destiny.sdk.CESdkException;
import com.nextlabs.destiny.sdk.CEUser;
import com.nextlabs.destiny.sdk.ICESdk;

/**
 * Created on Oct 9, 2017
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2017 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

public class NextLabsAuthorizationAgent {
	
	private static NextLabsAuthorizationAgent instance;
	private String hostname = null;
	private static CEApplication ceApp;
	private static CESdk ceSDK;
	private CEUser ceUser;
	private static boolean bConnected = false;
	
	// Policy Decision Point connection's details & default action
	private static String pdpHost;
	private static int pdpPort;
	private static String defaultAction;
	private static String defaultMessage;
	
	private NextLabsAuthorizationAgent(String host, int port, String action, String message) {
		pdpHost = host;
		pdpPort = port;
		
		defaultAction = action;
		defaultMessage = message;
	}
	
	/**
	 * Initializing the CE SDK
	 */
	private synchronized void initialize() {
		try {
			// retrieving the host name of the authorization agent
			hostname = getHostName();
			
			ceApp = new CEApplication(NextLabsPolicyConstants.CE_APPLICATION, NextLabsPolicyConstants.CE_NA);
			ceUser = new CEUser(NextLabsPolicyConstants.CE_USER, NextLabsPolicyConstants.CE_NA);
			
			try {
				ceSDK = new CESdk(pdpHost, pdpPort);
				bConnected = true;
			} catch (CESdkException ex) {
				System.out.println("NextLabsAuthorisationAgent.initialize() caught exception: " + ex.getMessage());
				bConnected = false;
			}
		} catch (UnknownHostException e) {
			System.out.println("initialize() caught exception: " + e.getMessage());
			bConnected = false;
		}
	}
	
	/**
	 * Getting an instance (singleton) of the NextLabsAuthorisationAgent
	 * @param pcHost
	 * @param pcPort
	 * @param defaultAction
	 * @param defaultMessage
	 * @return
	 */
	public static synchronized NextLabsAuthorizationAgent getInstance(String pcHost, int pcPort, 
			String defaultAction, String defaultMessage) {
		if (instance == null) {
			instance = new NextLabsAuthorizationAgent(pcHost, pcPort, defaultAction, defaultMessage);
			
			instance.initialize();
		}
		
		return instance;
	}
	
	/**
	 * For determine whether user is granted access to specific type
	 * @param username User name for the user that will be allow/deny access
	 * @param action Action performed by the user.
	 * @param attributes HashMap of attributes which contain all the object attributes that will be pass to NextLabs CE SDK
	 * @return HashMap of the response, contain response string and also the obligation information
	 * @throws Exception
	 */
	public HashMap<String, Object> hasAccess(String username, String action, 
			HashMap<String, Object> attributes, HashMap<String, Object> userAttributes,
			HashMap<String, Object> appAttributes) throws Exception {
		HashMap<String, Object> result = new HashMap<String, Object>();
		
		// Checking if handler is 0 then need to reconnect.
		synchronized (this) {
			if (!bConnected) {
				initialize();
			}
		}
		
		if (!bConnected) {
			System.out.println("Connection to CE SDK failed, will return default action which is " + 
					defaultAction);
			
			result.put(NextLabsPolicyConstants.RESPONSE_ERROR_KEY, "CE SDK Exception");
			result.put(NextLabsPolicyConstants.RESPONSE_KEY, defaultAction);
			
			if (defaultMessage != null) {
				result.put(NextLabsPolicyConstants.RESPONSE_MESSAGE_KEY, defaultMessage);
			}
			
			return result;
		}
		
		result = checkNxlPolicy(username, action, attributes, userAttributes, appAttributes);
		
		// If error happens then will direct return
		if (result.containsKey(NextLabsPolicyConstants.RESPONSE_ERROR_KEY))
			return result;
		else {
			// If this main object is being deny then will direct return
			if (!(NextLabsPolicyConstants.RESPONSE_ALLOW_VALUE).equalsIgnoreCase(
					(String) result.get(NextLabsPolicyConstants.RESPONSE_KEY)))
				return result;
		}
		
		return result;
	}
	
	/**
	 * Calling to NextLabs CE SDK to validate the access
	 * @param username User name for the user that will be allow/deny access
	 * @param action Action performed by the user.
	 * @param attributes HashMap of attributes which contain all the object attributes that will be pass to NextLabs CE SDK
	 * @return HashMap of the response, contain response string and also the obligation information
	 * @throws Exception
	 */
	public HashMap<String, Object> checkNxlPolicy(String username, String action, 
			HashMap<String, Object> attributes, HashMap<String, Object> userAttributes,
			HashMap<String, Object> appAttributes) throws Exception {
		
		HashMap<String, Object> result = new HashMap<String, Object>();
		
		try {			
			try {
				String ipaddress = (String) attributes.get("ip-address");
				
				long ipaddr;
				
				if (ipaddress == null) {
					ipaddr = 0;
				} else {
					if (isIPAddressValid(ipaddress)) {
						ipaddr = ipToInt(ipaddress);
					} else {
						ipaddr = 0;
					}
				}
				
				long lCurrentTime = System.nanoTime();
				

				CEResource source = getCESource(attributes);
				
				// Building the source attributes
				CEAttributes sourceAttr = new CEAttributes();
				
				sourceAttr.add(NextLabsPolicyConstants.ATTR_KEY_SERVER, hostname);
				sourceAttr.add(NextLabsPolicyConstants.ATTR_KEY_APPLICATION, NextLabsPolicyConstants.CE_APPLICATION);
				sourceAttr.add(NextLabsPolicyConstants.ATTR_KEY_NOCACHE, NextLabsPolicyConstants.ATTR_VALUE_YES);
				sourceAttr.add(NextLabsPolicyConstants.ATTR_KEY_URL, source.getName());
				
				// add operation-source
				sourceAttr.add(NextLabsPolicyConstants.ATTRIBUTE_OPERATION_SOURCE, NextLabsPolicyConstants.OPERATION_SOURCE);
				
				haspmapToCEAttrs(attributes, sourceAttr);
				
				// Building the user attributes
				CEAttributes userAttr = new CEAttributes();
				userAttributes.put("tc_username", username);
				haspmapToCEAttrs(userAttributes, userAttr);
				
				if (userAttr.size() <= 0)
					userAttr = null;
				
				// Building the application attributes
				CEAttributes appAttr = new CEAttributes();
				
				haspmapToCEAttrs(appAttributes, appAttr);
				
				if (appAttr.size() <= 0)
					appAttr = null;
				
				
				ceUser = new CEUser(username, username);
				System.out.println("CE username = " + ceUser.getName());
				
				String [] sAttr = sourceAttr.toArray();
				
				for (int i = 0; i < sAttr.length; i++) {
					System.out.println("Java calling  with |Data-->" + sAttr[i] + "   Values-->" + sAttr[i + 1] + "|");
					i++;
				}
				
				CEEnforcement enforcementResult = null;
				
				try {
					enforcementResult = ceSDK.checkResources(action, source, sourceAttr, null, null,
							ceUser, userAttr, ceApp, appAttr, null, null, (int) ipaddr, // ip address - local
							true, ICESdk.CE_NOISE_LEVEL_USER_ACTION, NextLabsPolicyConstants.PC_CONN_TIMEOUT);
				} catch (CESdkException e) {
					System.out.println("NextLabsAuthorizationAgent.checkNxlPolicy() caught exception: " + e.getMessage());
					e.printStackTrace(System.out);
					
					// If not cause by error connection, set ce handler to 0
					// so that connection will be re-initialize at again at next calls
					System.out.println("Connection failed");
					bConnected = false;
				}
				
				// If not able to get enforcement result will read from configuration file
				if (enforcementResult == null) {
					result.put(NextLabsPolicyConstants.RESPONSE_ERROR_KEY, "result is null");
					result.put(NextLabsPolicyConstants.RESPONSE_KEY, NextLabsPolicyConstants.DEFAULT_ACTION);
					
					if (null != NextLabsPolicyConstants.DEFAULT_MESSAGE) {
						result.put(NextLabsPolicyConstants.RESPONSE_MESSAGE_KEY, NextLabsPolicyConstants.DEFAULT_MESSAGE);
					}
					
					System.out.println("ERRORRESULT = " + result.get(NextLabsPolicyConstants.RESPONSE_KEY));
					
					return result;
				}
				
				System.out.println("CE response is " + enforcementResult.getResponseAsString() + 
						" with timing " + ((System.nanoTime() - lCurrentTime) / 1000000.00) + " ms");
				
				CEAttributes ceObligations = enforcementResult.getObligations();
				String[] sArrObligations = ceObligations.toArray();
				
				// For obligation handling
				for (int i = 0; i < sArrObligations.length; i = i + 2) {
					System.out.println("Obligation from CE is name:" + sArrObligations[i] + "--value:" + sArrObligations[i + 1]);
					result.put(sArrObligations[i], sArrObligations[i + 1]);
				}
				
				result.put(NextLabsPolicyConstants.RESPONSE_KEY, enforcementResult.getResponseAsString());
			} catch (Exception e) {
				System.out.println("NextLabsAuthorizationAgent.checkNxlPolicy() caught exception: " + e.getMessage());
				e.printStackTrace(System.out);
				
				result.put(NextLabsPolicyConstants.RESPONSE_ERROR_KEY, "NL SDK Exception.");
				result.put(NextLabsPolicyConstants.RESPONSE_KEY, NextLabsPolicyConstants.DEFAULT_ACTION);
				
				if (null != NextLabsPolicyConstants.DEFAULT_MESSAGE) {
					result.put(NextLabsPolicyConstants.RESPONSE_MESSAGE_KEY, NextLabsPolicyConstants.DEFAULT_MESSAGE);
				}
			}
		} catch (Exception ex) {
			System.out.println("NextLabsAuthorizationAgent.checkNxlPolicy() caught exception: " + ex.getMessage());
			ex.printStackTrace(System.out);
			
			throw ex;
		}	
		
		System.out.println("END of NextLabsAuthorizationAgent:checkNxlPolicy()");
		System.out.println("RESULT = " + result.get(NextLabsPolicyConstants.RESPONSE_KEY));
		
		return result;
	}
	
	private CEResource getCESource(HashMap<String, Object> attributes) {
		// use the Extension name and type when the keys are present
		String revType = (attributes.get(NextLabsPolicyConstants.ATTRIBUTE_REV_TYPE) != null) ? 
				(String) attributes.get(NextLabsPolicyConstants.ATTRIBUTE_REV_TYPE) : null;
		String revName = (attributes.get(NextLabsPolicyConstants.ATTRIBUTE_REV_NAME) != null) ? 
				(String) attributes.get(NextLabsPolicyConstants.ATTRIBUTE_REV_NAME) : null;
		String dsType = (attributes.get(NextLabsPolicyConstants.ATTRIBUTE_DS_TYPE) != null) ? 
				(String) attributes.get(NextLabsPolicyConstants.ATTRIBUTE_DS_TYPE) : null;
		String dsName = (attributes.get(NextLabsPolicyConstants.ATTRIBUTE_DS_NAME) != null) ? 
				(String) attributes.get(NextLabsPolicyConstants.ATTRIBUTE_DS_NAME) : null;
		
		// Building the resource string 
		String sResourceString = buildResourceString(hostname, revType, revName, dsType, dsName);
		
		return new CEResource(sResourceString, NextLabsPolicyConstants.ATTR_VALUE_PORTAL_TYPE);
	}
	
	/**
	 * Builds the resource URL in the following format:
	 * - 1. teamcenter://<host name>/<revision type>/<revision name>/<dataset type>/<dateset name>
	 * - 2. teamcenter://<host name>/<dataset type>/<dataset name>
	 * @param sHostName
	 * @param revType
	 * @param revName
	 * @param dsType
	 * @param dsName
	 * @return resource URL for policy evaluation
	 */
	protected String buildResourceString(String sHostName, String revType, String revName, String dsType, String dsName) {
		StringBuffer sBuf = new StringBuffer();
		sBuf.append("teamcenter://");
		sBuf.append(sHostName).append("/");
		
		if (revType == null && revName == null) {
			// nop
			// so the revision info will not be captured in resource url
		} else {
			if (revType == null)
				revType = "*";
			
			if (revName == null)
				revName = "*";
			
			sBuf.append(revType).append("/");
			sBuf.append(revName).append("/");
		}
		
		if (dsType == null)
			dsType = "*";

		if (dsName == null)
			dsName = "*";
		
		sBuf.append(dsType).append("/");
		sBuf.append(dsName);
		
		return sBuf.toString();
	}
	
	private void haspmapToCEAttrs(HashMap<String, Object> attributes, CEAttributes attrs) {
		for (Entry<String, Object> entry : attributes.entrySet()) {
			Object obj = entry.getValue();
			
			if (obj instanceof String) {
				String sValue = (String) obj;
				
				if (null != sValue && sValue.length() > 0) {
					attrs.add(entry.getKey(), sValue);
				}
			} else if (obj instanceof List<?>) {
				List<?> list = (List<?>) obj;
				
				for (Object listItem : list) {
					String sListItem = listItem.toString();
					
					if (null != sListItem && sListItem.length() > 0) {
						attrs.add(entry.getKey(), sListItem);
					}
				} // end of looping the list
			} else {
				// for example, use may put other object or boolean into the map
				String sValue = obj.toString();
				
				if (null != sValue && sValue.length() > 0) {
					attrs.add(entry.getKey(), sValue);
				}
			} // end of obj condition checking
		} // end of looping the attributes
	}
	
	/**
	 * Getting the host name for server currently running
	 * @return Host name for the server that running this application
	 * @throws Exception
	 */
	private static String getHostName() throws UnknownHostException {
		String hostname = null;
		
		try {
			InetAddress addr = InetAddress.getLocalHost();
			hostname = addr.getHostName();
		} catch (UnknownHostException e) {
			throw e;
		}
		
		return hostname;
	}
	
	/**
	 * Function to validate whether the IP is in valid form
	 * @param ipAddress IP address in string format e.g 127.0.0.1
	 * @return true if the IP address is valid, otherwise false
	 */
	public static boolean isIPAddressValid(String ipAddress) {
		String[] parts = ipAddress.split("\\.");
		
		if (parts.length != 4) {
			return false;
		}
		
		for (String s : parts) {
			int i = Integer.parseInt(s);
			
			if ((i < 0) || (i > 255)) {
				return false;
			}
		}
		
		return true;
	}
	
	/**
	 * Convert the IP from string form to integer form
	 * @param addr IP address in string format e.g 127.0.0.1
	 * @return IP address in integer format for example 0 for IP address 127.0.0.1
	 */
	public static long ipToInt(String addr) {
		String[] addrArray = addr.split("\\.");
		
		long num = 0;
		for (int i = 0; i < addrArray.length; i++) {
			int power = 3 - i;
			
			num += ((Integer.parseInt(addrArray[i]) % 256 * Math.pow(256, power)));
		}
		
		return num;
	}

}
