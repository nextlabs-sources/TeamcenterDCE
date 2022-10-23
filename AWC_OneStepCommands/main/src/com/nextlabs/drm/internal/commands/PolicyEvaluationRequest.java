package com.nextlabs.drm.internal.commands;


import java.util.HashMap;
import com.google.gwt.user.client.rpc.IsSerializable;

import com.nextlabs.drm.internal.config.NextLabsConstants;

/**
 * Data will be fetched from preferences and model object for policy evaluation request
 * @author clow
 *
 */
public class PolicyEvaluationRequest implements IsSerializable {

	//private static final long serialVersionUID = 2694409088980369745L;
	
	// connection details
	private String pdpHost = "";
	private int pdpPort = NextLabsConstants.DEFAULT_PDP_PORT;
	private String pdpDefaultAction = "";
	private String pdpDefaultMessage = "";
	
	
	// evaluation details
	private String user_id;
	private HashMap<String, Object> attributes;
	private HashMap<String, Object> userAttributes;
	private HashMap<String, Object> appAttributes;
	
	public PolicyEvaluationRequest() {
		
	}
	
	public PolicyEvaluationRequest(String pdpHost, int pdpPort, 
			String pdpDefaultAction, String pdpDefaultMessage) {
		this.pdpHost = pdpHost;
		this.pdpPort = pdpPort;
		this.pdpDefaultAction = pdpDefaultAction;
		this.pdpDefaultMessage = pdpDefaultMessage;
	}
	
	public PolicyEvaluationRequest(String pdpHost, int pdpPort, 
			String pdpDefaultAction, String pdpDefaultMessage, 
			String user_id, HashMap<String, Object> attributes, 
			HashMap<String, Object> userAttributes, HashMap<String, Object> appAttributes) {
		this.pdpHost = pdpHost;
		this.pdpPort = pdpPort;
		this.pdpDefaultAction = pdpDefaultAction;
		this.pdpDefaultMessage = pdpDefaultMessage;
		this.user_id = user_id;
		this.attributes = attributes;
		this.userAttributes = userAttributes;
		this.appAttributes = appAttributes;
	}
	
	public String getPdpHost() {
		return pdpHost;
	}

	public void setPdpHost(String pdpHost) {
		this.pdpHost = pdpHost;
	}
	
	public int getPdpPort() {
		return pdpPort;
	}

	public void setPdpPort(int pdpPort) {
		this.pdpPort = pdpPort;
	}

	public String getPdpDefaultAction() {
		return pdpDefaultAction;
	}

	public void setPdpDefaultAction(String pdpDefaultAction) {
		this.pdpDefaultAction = pdpDefaultAction;
	}

	public String getPdpDefaultMessage() {
		return pdpDefaultMessage;
	}

	public void setPdpDefaultMessage(String pdpDefaultMessage) {
		this.pdpDefaultMessage = pdpDefaultMessage;
	}

	public String getUser_id() {
		return user_id;
	}

	public void setUser_id(String user_id) {
		this.user_id = user_id;
	}

	public HashMap<String, Object> getAttributes() {
		return attributes;
	}

	public void setAttributes(HashMap<String, Object> attributes) {
		this.attributes = attributes;
	}

	public HashMap<String, Object> getUserAttributes() {
		return userAttributes;
	}

	public void setUserAttributes(HashMap<String, Object> userAttributes) {
		this.userAttributes = userAttributes;
	}

	public HashMap<String, Object> getAppAttributes() {
		return appAttributes;
	}

	public void setAppAttributes(HashMap<String, Object> appAttributes) {
		this.appAttributes = appAttributes;
	}
	
}
