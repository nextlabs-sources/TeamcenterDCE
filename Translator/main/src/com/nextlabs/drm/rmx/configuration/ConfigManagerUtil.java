package com.nextlabs.drm.rmx.configuration;

import java.io.File;
import java.net.URISyntaxException;

/**
 * Created on November 27, 2015
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

public class ConfigManagerUtil {
	
	public ConfigManagerUtil() {
		
	}
	
	public void encryptConfigPassword() throws URISyntaxException {
		File baseFile = new File(ClassLoader.getSystemClassLoader().getResource(".").toURI());
		
		try {
			ConfigManager configManager = new ConfigManager.Builder(baseFile).buildConfig();
			configManager.getConfigPropertiesDTO();
			
			System.out.println("ConfigManagerUtil::encryptConfigPassword is done");
		} catch (Exception ex) {
			System.out.println("ConfigManagerUtil::encryptConfigPassword caught exception: " + ex.getMessage());
			ex.printStackTrace(System.out);
		}
	}
	
	public static void main(String[] args) {
		ConfigManagerUtil configManagerUtil = new ConfigManagerUtil();
		
		try {
			configManagerUtil.encryptConfigPassword();
		} catch (URISyntaxException e) {
			System.out.println("error: " + e.getMessage());
		}
	}
	
}
