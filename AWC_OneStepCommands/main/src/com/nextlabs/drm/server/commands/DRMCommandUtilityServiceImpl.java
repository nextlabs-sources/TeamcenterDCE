/*
 * Created on October 25, 2017
 *
 * All sources, binaries and HTML pages (C) copyright 2017 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.drm.server.commands;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import com.google.gwt.user.server.rpc.RemoteServiceServlet;
import com.nextlabs.drm.internal.commands.DRMCommandUtilityService;
import com.nextlabs.drm.internal.commands.PolicyEvaluationRequest;
import com.nextlabs.drm.internal.config.NextLabsConstants;
import com.nextlabs.drm.server.policy.NextLabsAuthorizationAgent;
import com.nextlabs.drm.server.policy.NextLabsPolicyConstants;

public class DRMCommandUtilityServiceImpl extends RemoteServiceServlet 
		implements DRMCommandUtilityService, NextLabsConstants {
	
	private static final long serialVersionUID = 8536897894828298909L;
	private NextLabsAuthorizationAgent agent;
	private String pdpDefaultAction = "";
	private String pdpDefaultMessage = "";
	
	public DRMCommandUtilityServiceImpl() {
		
	}
	
	/**
	 * Policy evaluation for certain to remove protection. 
	 * result: true for Allow; false for Deny
	 */
	public Boolean policyEvaluation(PolicyEvaluationRequest request) {
		if (agent == null) {
			// setting up NextLabsAuthorizationAgent by fetching connection details
			// from TC preferences
			try {
				agent = NextLabsAuthorizationAgent.getInstance(request.getPdpHost(), request.getPdpPort(), 
							pdpDefaultAction, pdpDefaultMessage);
			} catch (Exception ex) {
				System.out.println("DRMCommandUtilityServiceImpl.policyEvaluation() caught exception: " + ex.getMessage());
				ex.printStackTrace(System.out);
				
				// return default action when agent failed to initiate
				if (!NextLabsPolicyConstants.RESPONSE_ALLOW_VALUE.equalsIgnoreCase(pdpDefaultAction))
					return Boolean.FALSE;
			}
		}
		
		// print out the PDP connection details
		System.out.println("DRMCommandUtilityServiceImpl PDP connection details: ");
		System.out.println("> " + request.getPdpHost());
		System.out.println("> " + request.getPdpPort());
		System.out.println("> " + request.getPdpDefaultAction());
		System.out.println("> " + request.getPdpDefaultMessage());
		HashMap<String, Object> attributes = request.getAttributes();
		
		Iterator<Map.Entry<String, Object>> it = attributes.entrySet().iterator();
		while (it.hasNext()) {
			Map.Entry<String, Object> entry = (Map.Entry<String, Object>) it.next();
			System.out.println("> " + entry.getKey() + " : " + entry.getValue().toString());
		}
		
		try {
			HashMap<String, Object> hasAccessResult = agent.hasAccess(request.getUser_id(), 
					NextLabsPolicyConstants.RIGHT_EXECUTE_REMOVEPROTECTION, attributes, 
					new HashMap<String, Object>(), new HashMap<String, Object>());
			
			String response = (String) hasAccessResult.get(NextLabsPolicyConstants.RESPONSE_KEY);
			String sError = (String) hasAccessResult.get(NextLabsPolicyConstants.RESPONSE_ERROR_KEY);
			
			// Checking to see if there is SDK error response
			if (null != sError) {
				String sDefaultMsg = (String) hasAccessResult.get(NextLabsPolicyConstants.RESPONSE_MESSAGE_KEY);
				
				if (null != sDefaultMsg) {
					System.out.println("CE SDK error: " + sDefaultMsg);
				}
			}
			
			// fix bug 46950
			String obligationCount = (String) hasAccessResult.get(NextLabsPolicyConstants.CE_ATTR_OBLIGATION_COUNT);
			boolean obligationRemoveProtection = false;
			
			if (obligationCount != null) {
				try {
					int count = Integer.parseInt(obligationCount);
					
					for (int i = 1; i <= count; i++) {
						String obligationName = (String) 
								hasAccessResult.get(NextLabsPolicyConstants.CE_ATTR_OBLIGATION_NAME + ":" + i);
						
						if (obligationName != null && 
								obligationName.equalsIgnoreCase(NextLabsPolicyConstants.OBLIGATION_REMOVEPROTECTION)) {
							obligationRemoveProtection = true;
							break;
						}
					}
				} catch (NumberFormatException nfe) {
					System.out.println("DRMHandler.processDataset() caught exception: " + nfe.getMessage());
					nfe.printStackTrace(System.out);
				}
			}
			
			if (!NextLabsPolicyConstants.RESPONSE_ALLOW_VALUE.equalsIgnoreCase(response) || 
					!obligationRemoveProtection)
				return Boolean.FALSE;
		} catch (Exception ex) {
			System.out.println("DRMCommandUtilityServiceImpl.policyEvaluation() caught exception: " + ex.getMessage());
			ex.printStackTrace(System.out);
			
			if (!NextLabsPolicyConstants.RESPONSE_ALLOW_VALUE.equalsIgnoreCase(pdpDefaultAction))
				return Boolean.FALSE;
		}
		
		return Boolean.TRUE;
	}

	
}
