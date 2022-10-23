package com.nextlabs.drm.tonxlfile.configuration;

/**
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2019 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

import com.teamcenter.soa.client.Connection;

public class NextLabsConfigManager {
	
	public NextLabsConfigManager() {
		// default constructor
	}
	
	public NextLabsConfigInterface getService(Connection conn) {
		return new TeamcenterPreferences(conn);
	}
	
}
