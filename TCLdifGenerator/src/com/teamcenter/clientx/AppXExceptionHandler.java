//==================================================
//
//  Copyright 2012 Siemens Product Lifecycle Management Software Inc. All Rights Reserved.
//
//==================================================

package com.teamcenter.clientx;

import com.teamcenter.schemas.soa._2006_03.exceptions.ConnectionException;
import com.teamcenter.schemas.soa._2006_03.exceptions.InternalServerException;
import com.teamcenter.schemas.soa._2006_03.exceptions.ProtocolException;
import com.teamcenter.soa.client.ExceptionHandler;
import com.teamcenter.soa.exceptions.CanceledOperationException;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * Implementation of the ExceptionHandler. For ConnectionExceptions (server
 * temporarily down .etc) prompts the user to retry the last request. For other
 * exceptions convert to a RunTime exception.
 */
public class AppXExceptionHandler implements ExceptionHandler {
	private static final Logger logger                      = LogManager.getLogger("TCLDIFLOGGER");

	/*
	 * (non-Javadoc)
	 * 
	 * @see com.teamcenter.soa.client.ExceptionHandler#handleException(com.teamcenter.schemas.soa._2006_03.exceptions.InternalServerException)
	 */
	public void handleException(InternalServerException ise) {
		logger.info("*****");
		logger.info("Exception caught in com.teamcenter.clientx.AppXExceptionHandler.handleException(InternalServerException).");

		if (ise instanceof ConnectionException) {
			// ConnectionException are typically due to a network error (server
			// down .etc) and can be recovered from (the last request can be
			// sent again,
			// after the problem is corrected).
			logger.error("\nThe server returned an connection error.\n"
					+ ise.getMessage()
					+ "\nDo you wish to retry the last service request?[y/n]");
		} else if (ise instanceof ProtocolException) {
			// ProtocolException are typically due to programming errors
			// (content of HTTP
			// request is incorrect). These are generally can not be
			// recovered from.
			logger.error("\nThe server returned an protocol error.\n"
							+ ise.getMessage()
							+ "\nThis is most likely the result of a programming error."
							+ "\nDo you wish to retry the last service request?[y/n]");
		} else {
			logger.error("\nThe server returned an internal server error.\n"
							+ ise.getMessage()
							+ "\nThis is most likely the result of a programming error."
							+ "\nA RuntimeException will be thrown.");
			throw new RuntimeException(ise.getMessage());
		}

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * com.teamcenter.soa.client.ExceptionHandler#handleException(com.teamcenter
	 * .soa.exceptions.CanceledOperationException)
	 */
	public void handleException(CanceledOperationException coe) {
		logger.info("*****");
		logger.info("Exception caught in com.teamcenter.clientx.AppXExceptionHandler.handleException(CanceledOperationException).");

		// Expecting this from the login tests with bad credentials, and the
		// AnyUserCredentials class not
		// prompting for different credentials
		throw new RuntimeException(coe);
	}

}
