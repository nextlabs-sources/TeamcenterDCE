package com.nextlabs.drm.rmx.batchprocess.tcsession;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.teamcenter.soa.client.RequestListener;

public class TCRequestListener implements RequestListener {
	
	private final static Logger LOGGER = LogManager.getLogger("BPLOGGER");
	
	@Override
	public void serviceRequest(Info info) {
		LOGGER.debug("TCRequestListener.serviceRequest() " + info.id + ": " + info.service + "." + info.operation);
	}
	
	@Override
	public void serviceResponse(Info info) {
		LOGGER.debug("TCRequestListener.serviceResponse() " + info.id + ": " + info.service + "." + info.operation);		
	}

}
