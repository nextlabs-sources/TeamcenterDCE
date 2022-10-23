package com.nextlabs.drm.rmx.configuration;

import java.io.File;
import java.net.URISyntaxException;

public class SecretUtil {
	
	public SecretUtil() {
		
	}
	
	public void encryptSecret() throws URISyntaxException {
		File baseFile = new File(ClassLoader.getSystemClassLoader().getResource(".").toURI());
		
		try {
			ConfigManager configManager = new ConfigManager.Builder(baseFile).buildSecret();
			configManager.getSecretPropertiesDTO();
			
			System.out.println("PasswordUtil::encryptSecret is done");
		} catch (Exception ex) {
			System.out.println("PasswordUtil::encryptSassword caught exception: " + ex.getMessage());
			ex.printStackTrace(System.out);
		}
	}
	
	public static void main(String[] args) {
		SecretUtil secretUtil = new SecretUtil();
		
		try {
			secretUtil.encryptSecret();
		} catch (URISyntaxException e) {
			System.out.println("error: " + e.getMessage());
		}
	}

}
