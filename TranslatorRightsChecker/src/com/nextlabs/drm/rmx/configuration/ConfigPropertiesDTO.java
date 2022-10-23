package com.nextlabs.drm.rmx.configuration;

/**
 * Created on February 13, 2019
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

public class ConfigPropertiesDTO {
	
	private final String routerURL;
	private final String tenantName;
	private final int appID;
	private final String appKey;
	
	public ConfigPropertiesDTO(String routerURL, String tenantName, int appID, String appKey) {
		this.routerURL = routerURL;
		this.tenantName = tenantName;
		this.appID = appID;
		this.appKey = appKey;
	}

	public String getRouterURL() {
		return routerURL;
	}

	public String getTenantName() {
		return tenantName;
	}

	public int getAppID() {
		return appID;
	}

	public String getAppKey() {
		return appKey;
	}

}
