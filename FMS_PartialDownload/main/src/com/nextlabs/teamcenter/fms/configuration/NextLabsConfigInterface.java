/*
 * Created on November 9, 2017
 * Updated on February 25, 2019: Supports SkyDRM
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.configuration;

public interface NextLabsConfigInterface {
	
	/**
	 * Returns whether the config interface has data loaded or not.
	 * @return (boolean) <code>true</code> if the data has been loaded.
	 */
	public boolean hasData();
	
	// SkyDRM configuration
	/**
	 * Returns the router URL of SkyDRM from the configuration.
	 * @return (String) The router URL of SkyDRM.
	 */
	public String getRouterURL();
	
	/**
	 * Returns the tenant name of SkyDRM from the configuration.
	 * @return (String) The tenant name of SkyDRM for DCE.
	 */
	public String getTenantName();
	
	/**
	 * Returns the App ID of SkyDRM from the configuration.
	 * @return (int) The APP ID of SkyDRM for DCE.
	 */
	public int getAppID();
	
	/**
	 * Returns the App Key of SkyDRM from the configuration.
	 * @return (String) The App Key of SkyDRM for DCE.
	 */
	public String getAppKey();

}
