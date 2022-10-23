package com.nextlabs.drm.rmx.batchprocess.tcsession;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.teamcenter.soa.client.model.ErrorStack;
import com.teamcenter.soa.client.model.ErrorValue;
import com.teamcenter.soa.client.model.PartialErrorListener;

public class TCPartialErrorListener implements PartialErrorListener {
	
	private final static Logger LOGGER = LogManager.getLogger("BPLOGGER");
	
	@Override
	public void handlePartialError(ErrorStack[] stacks) {
		if (stacks.length == 0) return;
		
		LOGGER.error("TCPartialErrorListener.handlePartialError() caughts errors.");
		for (int i = 0; i < stacks.length; i++) {
			ErrorValue[] errors = stacks[i].getErrorValues();
			LOGGER.error("  Partial error for ");
			
			// The different service implementation may optionally associate
			// an ModelObject, client ID, or nothing, with each partial error
			if (stacks[i].hasAssociatedObject())
				LOGGER.error("    object " + stacks[i].getAssociatedObject().getUid());
			else if (stacks[i].hasClientId())
				LOGGER.error("    client id " + stacks[i].getClientId());
			else if (stacks[i].hasClientIndex())
				LOGGER.error("    client index " + stacks[i].getClientIndex());
			
			// Each partial error will have one or more contributing error messages
			for (int j = 0; j < errors.length; j++)
				LOGGER.error("    code: " + errors[j].getCode() + " severity: " + 
						errors[j].getLevel() + " " + errors[j].getMessage());
						
		}
	}

}
