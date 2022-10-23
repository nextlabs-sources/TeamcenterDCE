package com.nextlabs.drm.rmx.batchprotect.tcsession;

/**
 * Created on December 2, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import com.teamcenter.soa.SoaConstants;
import com.teamcenter.soa.client.Connection;

public class TCSession {

	private Connection connection;
	private TCCredentialManager credentialManager;

	/**
	 * Create an instance of the Session with a connection to the specified
	 * server.
	 */
	public TCSession(String host, String userId, String token, String discriminator, 
			String group, String role) {
		// Create an instance of the CredentialManager, this is used
		// by the SOA Framework to get the user's credentials when
		// challenged by the server (session timeout on the web tier).
		credentialManager = new TCCredentialManager();

		credentialManager.setUserPassword(userId, token, discriminator);
		credentialManager.setGroupRole(group, role);

		String protocol = SoaConstants.HTTP;
		
		// Create the Connection object, no contact is made with the server
		// until a service request is made
		connection = new Connection(host, credentialManager, SoaConstants.REST,
				protocol);

		// Add an ExceptionHandler to the Connection, this will handle any
		// InternalServerException, communication errors, XML marshaling errors
		// .etc
		connection.setExceptionHandler(new TCExceptionHandler());

	}

	/**
	 * Get the Connection object
	 * 
	 * @return connection
	 */
	public Connection getConnection() {
		return connection;
	}

}
