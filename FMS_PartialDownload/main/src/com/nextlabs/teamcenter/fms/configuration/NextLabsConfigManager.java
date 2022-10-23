/*
 * Created on November 9, 2017
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.configuration;

public class NextLabsConfigManager {
	
	/** The singleton instance for NextLabsConfigManager. */
	private static NextLabsConfigManager instance = null;
	
	/**
	 * Private constructor
	 */
	private NextLabsConfigManager() {
		// private default constructor
	}
	
	/**
	 * 
	 * @return The NextLabsConfigManager instance.
	 */
	public static synchronized NextLabsConfigManager getService() {
		if (instance == null) 
			instance = new NextLabsConfigManager();
			
		return instance;
	}
	
	/**
	 * 
	 * @return An instance of NextLabsConfigInterface
	 */
	public NextLabsConfigInterface getConfiguration() {
		NextLabsConfigInterface configuration = new NXLPropertiesFile();
		
		return configuration;
	}

}
