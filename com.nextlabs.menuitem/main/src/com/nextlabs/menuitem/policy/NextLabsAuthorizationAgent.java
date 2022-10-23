package com.nextlabs.menuitem.policy;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Properties;
import java.util.Map.Entry;

import org.apache.openaz.pepapi.Action;
import org.apache.openaz.pepapi.Obligation;
import org.apache.openaz.pepapi.PepAgent;
import org.apache.openaz.pepapi.PepAgentFactory;
import org.apache.openaz.pepapi.PepResponse;
import org.apache.openaz.pepapi.Resource;
import org.apache.openaz.pepapi.Subject;
import org.apache.openaz.pepapi.std.StdPepAgentFactory;
import org.apache.openaz.xacml.api.Decision;
import org.apache.openaz.xacml.util.XACMLProperties;

import com.nextlabs.menuitem.configuration.SimpleCrypto;

import static com.nextlabs.openaz.utils.Constants.*;
import static com.nextlabs.menuitem.policy.NextLabsPolicyConstants.*;

public class NextLabsAuthorizationAgent {
	
	private static NextLabsAuthorizationAgent instance;
	private String hostname = null;
	private static PepAgent pepAgent;
	private Subject user;
	private static boolean bConnected = false;
	
	// Policy Decision Point connection's details & default action
	private static String pdpHost;
	private static String pdpPort;
	private static String pdpHttps;
	private static String oauth2Host;
	private static String oauth2Port;
	private static String oauth2Https;
	private static String oauth2ClientId;
	private static String oauth2ClientSecret;
	private static String pdpIgnoreHttps;
	private static String defaultAction;
	private static String defaultMessage;
	
	private static final int CONN_RETRY = 2;
		
	private NextLabsAuthorizationAgent(String pdpHostIn, String pdpPortIn, String pdpHttpsIn, 
			String pdpIgnoreHttpsIn, String oauth2HostIn, String oauth2PortIn, String oauth2HttpsIn, 
			String oauth2ClientIdIn, String oauth2ClientSecretIn, String defaultActionIn, String defaultMessageIn) {
		pdpHost = pdpHostIn;
		pdpPort = pdpPortIn;
		pdpHttps = pdpHttpsIn;
		pdpIgnoreHttps = pdpIgnoreHttpsIn;
		
		oauth2Host = oauth2HostIn;
		oauth2Port = oauth2PortIn;
		oauth2Https = oauth2HttpsIn;
		oauth2ClientId = oauth2ClientIdIn;
		oauth2ClientSecret = oauth2ClientSecretIn;
		
		defaultAction = defaultActionIn;
		defaultMessage = defaultMessageIn;
	}
	
	/**
	 * Initializing the CE SDK
	 */
	private synchronized void initialize() {
		SimpleCrypto simpleCrypto = new SimpleCrypto();
		
		try {
			// retrieving the host name of the authorization agent
			hostname = getHostName();
			
			user = Subject.newInstance(CE_USER);
			user.addAttribute("user_id", CE_USER);
			
			Properties xacmlProperties = new Properties();
			xacmlProperties.setProperty(ENGINE_NAME, TCRMX_PDP_ENGINE);
			xacmlProperties.setProperty(PDP_REST_PAYLOAD_CONTENT_TYPE, TCRMX_PAYLOAD_JSON);
			xacmlProperties.setProperty(PDP_REST_HOST, pdpHost); // host *
			xacmlProperties.setProperty(PDP_REST_PORT, pdpPort); // port *
			xacmlProperties.setProperty(PDP_REST_HTTPS, pdpHttps); // https * 
			xacmlProperties.setProperty(PDP_REST_AUTH_TYPE, TCRMX_OAUTH2);
			xacmlProperties.setProperty(PDP_REST_OAUTH2_TOKEN_GRANT_TYPE, TCRMX_TOKEN_TYPE);
			xacmlProperties.setProperty(PDP_REST_OAUTH2_SERVER, oauth2Host); // host2 *
			xacmlProperties.setProperty(PDP_REST_OAUTH2_PORT, oauth2Port); // port2 *
			xacmlProperties.setProperty(PDP_REST_OAUTH2_HTTPS, oauth2Https); // https2 *
			xacmlProperties.setProperty(PDP_REST_OAUTH2_CLIENT_ID, oauth2ClientId); // clientid *
			xacmlProperties.setProperty(PDP_REST_OAUTH2_CLIENT_SECRET, simpleCrypto.decrypt(oauth2ClientSecret)); // client sercret (encrypted) *
			xacmlProperties.setProperty(PDP_REST_OAUTH2_TOKEN_ENDPOINT_PATH, TCRMX_TOKEN_PATH);
			xacmlProperties.setProperty(PDP_REST_IGNORE_HTTPS_CERTIFICATE, pdpIgnoreHttps); // *
			xacmlProperties.setProperty(XACMLProperties.PROP_PDPENGINEFACTORY, TCRMX_PDP_ENGINEFAC);
			xacmlProperties.setProperty(TCRMX_MAPPER_KEY, TCRMX_MAPPER_VAL);
			 
			PepAgentFactory pepAgentFactory = new StdPepAgentFactory(xacmlProperties);
			pepAgent = pepAgentFactory.getPepAgent();
			
			bConnected = true;
		} catch (Exception e) {
			System.out.println("NextLabsAuthorisationAgent.initialize() caught exception: " + e.getMessage());
			bConnected = false;
		}
	}
		
	public static synchronized NextLabsAuthorizationAgent getInstance(String pdpHostIn, 
			String pdpPortIn, String pdpHttpsIn, String pdpIgnoreHttpsIn, String oauth2HostIn, 
			String oauth2PortIn, String oauth2HttpsIn, String oauth2ClientIdIn, 
			String oauth2ClientSecretIn, String defaultActionIn, String defaultMessageIn) {
		if (instance == null) {
			instance = new NextLabsAuthorizationAgent(pdpHostIn, pdpPortIn, pdpHttpsIn, 
					pdpIgnoreHttpsIn, oauth2HostIn, oauth2PortIn, oauth2HttpsIn, 
					oauth2ClientIdIn, oauth2ClientSecretIn, defaultActionIn, defaultMessageIn);
			
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
		int connRetryCount = CONN_RETRY;
		
		do {
			// Checking if handler is 0 then need to reconnect.
			synchronized (this) {
				if (!bConnected)
					initialize();
			}
			
			if (!bConnected && connRetryCount <= 0) {
				System.out.println("Connection to CE SDK failed, will return default action which is " + 
						defaultAction);
				
				result.put(RESPONSE_ERROR_KEY, "CE SDK Exception");
				result.put(RESPONSE_KEY, defaultAction);
				
				if (defaultMessage != null)
					result.put(RESPONSE_MESSAGE_KEY, defaultMessage);
				
				return result;
			}
			
			result = checkNxlPolicy(username, action, attributes, userAttributes, appAttributes);
			
			// If error is timeout/conn problem, retry to connect
		} while (result.containsKey(RESPONSE_ERROR_KEY) 
					&& !bConnected 
					&& connRetryCount-- > 0);
		
		// If error happens then will direct return
		if (result.containsKey(RESPONSE_ERROR_KEY))
			return result;
		else {
			// If this main object is being deny then will direct return
			if (!(RESPONSE_ALLOW_VALUE).equalsIgnoreCase(
					(String) result.get(RESPONSE_KEY)))
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
	public HashMap<String, Object> checkNxlPolicy(String username, String strAction, 
			HashMap<String, Object> attributes, HashMap<String, Object> userAttributes,
			HashMap<String, Object> appAttributes) throws Exception {
		
		HashMap<String, Object> result = new HashMap<String, Object>();
		
		try {
			long lCurrentTime = System.nanoTime();
			

			String resourceString = getResourceString(attributes);
							
			user = Subject.newInstance(username);
			user.addAttribute("user_id", username);
			user.addAttribute("tc_username", username);
			System.out.println("CE userid = " + username);
			
			Action action = Action.newInstance("RIGHT_EXECUTE_REMOVEPROTECTION");
			
			Resource resource = Resource.newInstance(ATTR_VALUE_PORTAL_TYPE);
			resource.addAttribute(ID_RESOURCE_RESOURCE_TYPE.stringValue(), ATTR_VALUE_PORTAL_TYPE);
			resource.addAttribute(ATTR_KEY_HOST, hostname);
			resource.addAttribute(ATTR_KEY_APPLICATION, CE_APPLICATION);
			resource.addAttribute(ATTR_KEY_URL, resourceString);
			resource.addAttribute(ATTRIBUTE_OPERATION_SOURCE, OPERATION_SOURCE);
			
			hashmapToResource(attributes, resource);
			
			PepResponse pepResponse = null;
			
			try {
				System.out.println("Checking access with PDP...");
				pepResponse = pepAgent.decide(user, action, resource);
			} catch (Exception e) {
				System.out.println("NextLabsAuthorizationAgent.checkNxlPolicy() caught exception: " + e.getMessage());
				e.printStackTrace(System.out);
				
				// If not cause by error connection, set ce handler to 0
				// so that connection will be re-initialize at again at next calls
				//System.out.println("Connection failed");
				bConnected = false;
			}
			
			// If not able to get enforcement result will read from configuration file
			if (pepResponse == null) {
				result.put(RESPONSE_ERROR_KEY, "result is null");
				result.put(RESPONSE_KEY, DEFAULT_ACTION);
				
				if (null != DEFAULT_MESSAGE)
					result.put(RESPONSE_MESSAGE_KEY, DEFAULT_MESSAGE);
				
				System.out.println("ERRORRESULT = " + result.get(RESPONSE_KEY));
				
				return result;
			}
			
			Decision decision = pepResponse.getWrappedResult().getDecision();
			String effectValue = (decision == Decision.PERMIT) ? "Allow" : decision.toString();
			
			System.out.println("CE response is " + effectValue + 
					" with timing " + ((System.nanoTime() - lCurrentTime) / 1000000.00) + " ms");
			
			if (!pepResponse.getObligations().isEmpty()) {
				Iterator<Entry<String, Obligation>> obligationEntryIter = pepResponse.getObligations().entrySet()
						.iterator();
				
				int i = 1;
				while (obligationEntryIter.hasNext()) {
					Entry<String, Obligation> obligationEntry = obligationEntryIter.next();						
					System.out.println(CE_ATTR_OBLIGATION_NAME + ":" + i + ", " + obligationEntry.getKey());
					result.put(CE_ATTR_OBLIGATION_NAME + ":" + i++, obligationEntry.getKey());
				}
				result.put(CE_ATTR_OBLIGATION_COUNT, (i - 1));
				System.out.println(CE_ATTR_OBLIGATION_COUNT + ", "+  (i - 1));
			}
			
			result.put(RESPONSE_KEY, effectValue);
		} catch (Exception e) {
			System.out.println("NextLabsAuthorizationAgent.checkNxlPolicy() caught exception: " + e.getMessage());
			e.printStackTrace(System.out);
			
			result.put(RESPONSE_ERROR_KEY, "NL SDK Exception.");
			result.put(RESPONSE_KEY, DEFAULT_ACTION);
			
			if (null != DEFAULT_MESSAGE) {
				result.put(RESPONSE_MESSAGE_KEY, DEFAULT_MESSAGE);
			}
		}
		
		System.out.println("END of NextLabsAuthorizationAgent:checkNxlPolicy()");
		System.out.println("RESULT = " + result.get(RESPONSE_KEY));
		
		return result;
	}
	
	private String getResourceString(HashMap<String, Object> attributes) {
		// use the Extension name and type when the keys are present
		String revType = (attributes.get(ATTRIBUTE_REV_TYPE) != null) ? 
				(String) attributes.get(ATTRIBUTE_REV_TYPE) : null;
		String revName = (attributes.get(ATTRIBUTE_REV_NAME) != null) ? 
				(String) attributes.get(ATTRIBUTE_REV_NAME) : null;
		String dsType = (attributes.get(ATTRIBUTE_DS_TYPE) != null) ? 
				(String) attributes.get(ATTRIBUTE_DS_TYPE) : null;
		String dsName = (attributes.get(ATTRIBUTE_DS_NAME) != null) ? 
				(String) attributes.get(ATTRIBUTE_DS_NAME) : null;
		
		// Building the resource string 
		return buildResourceString(hostname, revType, revName, dsType, dsName);
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
	
	private void hashmapToResource(HashMap<String, Object> attributes, Resource resource) {
		for (Entry<String, Object> entry : attributes.entrySet()) {
			Object obj = entry.getValue();
			
			if (obj instanceof String) {
				String sValue = (String) obj;
				
				if (null != sValue && sValue.length() > 0)
					resource.addAttribute(entry.getKey(), sValue);
			} else if (obj instanceof List<?>) {
				List<?> list = (List<?>) obj;
				List<String> lstString = new ArrayList<String>();
				
				for (Object listItem : list) {
					String sListItem = listItem.toString();
					
					if (null != sListItem && sListItem.length() > 0)
						lstString.add(sListItem);
				} // end of looping the list
				
				resource.addAttribute(entry.getKey(), lstString.toArray(new String[lstString.size()]));
			} else {
				// for example, use may put other object or boolean into the map
				String sValue = obj.toString();
				
				if (null != sValue && sValue.length() > 0)
					resource.addAttribute(entry.getKey(), sValue);
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

		InetAddress addr = InetAddress.getLocalHost();
		hostname = addr.getHostName();

		return hostname;
	}

}
