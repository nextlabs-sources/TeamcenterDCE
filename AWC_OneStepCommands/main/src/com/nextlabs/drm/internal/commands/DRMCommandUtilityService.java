package com.nextlabs.drm.internal.commands;

import com.google.gwt.user.client.rpc.RemoteService;
import com.google.gwt.user.client.rpc.RemoteServiceRelativePath;

@RemoteServiceRelativePath("drmCommandUtility")
public interface DRMCommandUtilityService extends RemoteService {

	/**
	 * Policy evaluation for certain to remove protection. result: true for
	 * Allow; false for Deny
	 */
	public Boolean policyEvaluation(PolicyEvaluationRequest request);
	
}
