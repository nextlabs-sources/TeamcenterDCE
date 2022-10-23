//==================================================
//
//  Copyright 2012 Siemens Product Lifecycle Management Software Inc. All Rights Reserved.
//
//==================================================

package com.teamcenter.clientx;

import com.teamcenter.soa.client.model.ErrorStack;
import com.teamcenter.soa.client.model.ErrorValue;
import com.teamcenter.soa.client.model.PartialErrorListener;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * Implementation of the PartialErrorListener. Print out any partial errors
 * returned.
 *
 */
public class AppXPartialErrorListener implements PartialErrorListener {

	private static final Logger logger                      = LogManager.getLogger("TCLDIFLOGGER");

	@Override
	public void handlePartialError(ErrorStack[] stacks) {
		if (stacks.length == 0)
			return;

		logger.info("");
		logger.info("*****");
		logger.error("Partial Errors caught in com.teamcenter.clientx.AppXPartialErrorListener.");

		for (int i = 0; i < stacks.length; i++) {
			ErrorValue[] errors = stacks[i].getErrorValues();
			logger.error("Partial Error for ");

			// The different service implementation may optionally associate
			// an ModelObject, client ID, or nothing, with each partial error
			if (stacks[i].hasAssociatedObject()) {
				logger.error("object " + stacks[i].getAssociatedObject().getUid());
			} else if (stacks[i].hasClientId()) {
				logger.error("client id " + stacks[i].getClientId());
			} else if (stacks[i].hasClientIndex()) {
				logger.error("client index " + stacks[i].getClientIndex());
			}

			// Each Partial Error will have one or more contributing error
			// messages
			for (int j = 0; j < errors.length; j++) {
				logger.error("    Code: " + errors[j].getCode()
						+ "\tSeverity: " + errors[j].getLevel() + "\t"
						+ errors[j].getMessage());
			}
		}

	}

}
