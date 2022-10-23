package com.nextlabs.drm.rmx.batchprocess.tcsession;

/**
 * Created on December 2, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.teamcenter.schemas.soa._2006_03.exceptions.ConnectionException;
import com.teamcenter.schemas.soa._2006_03.exceptions.InternalServerException;
import com.teamcenter.schemas.soa._2006_03.exceptions.ProtocolException;
import com.teamcenter.soa.client.ExceptionHandler;
import com.teamcenter.soa.exceptions.CanceledOperationException;

/**
 * Implementation of the ExceptionHandler. For ConnectionExceptions (server
 * temporarily down .etc) prompts the user to retry the last request. For other
 * exceptions convert to a RunTime exception.
 */
public class TCExceptionHandler implements ExceptionHandler {
	
	private final static Logger LOGGER = LogManager.getLogger("BPLOGGER");

	public void handleException(InternalServerException ise) {
		if (ise instanceof ConnectionException) {
			// ConnectionException are typically due to a network error (server
			// down .etc) and can be recovered from (the last request can be sent again,
			// after the problem is corrected).
			LOGGER.error("TCExceptionHandler.handleException() caughts connetion error: " + ise.getMessage(), ise);
		} else if (ise instanceof ProtocolException) {
			// ProtocolException are typically due to programming errors
			// (content of HTTP request is incorrect). 
			// These are generally can not be recovered from.
			LOGGER.error("TCExceptionHandler.handleException() caughts protocol error: " + ise.getMessage(), ise);
		} else {			
			LOGGER.error("TCExceptionHandler.handleException() caughts internal server error: " + ise.getMessage(), ise);
		}

		throw new RuntimeException(ise.getMessage());
	}

	public void handleException(CanceledOperationException coe) {
		LOGGER.error("TCExceptionHandler.handleException() caughts canceled operation error: " + coe.getMessage());
		
		throw new RuntimeException(coe);
	}

}
