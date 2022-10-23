package com.nextlabs.drm.internal.commands;

import com.google.gwt.user.client.rpc.AsyncCallback;

interface DRMCommandUtilityServiceAsync {

	/**
	 * Policy evaluation for certain to remove protection. result: true for
	 * Allow; false for Deny
	 */
	public void policyEvaluation(PolicyEvaluationRequest request, 
			AsyncCallback<Boolean> callback);
	
}
