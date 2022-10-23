package com.nextlabs.drm.rmx.configuration;

/*
 * Created on November 27, 2015
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Properties;

public class ConfigManager {

	public static final String ROUTER_URL = "ROUTER_URL";
	public static final String TENANT_NAME = "TENANT_NAME";
	public static final String APP_ID = "APP_ID";
	public static final String APP_KEY = "APP_KEY";
	
	public static final String SECRET = "SECRET";

	private static final String PREFIX_CRYPT = "crypt:";
	private static final String CONFIG_FILE = "config.properties";
	private static final String PASSWORD_FILE = "secret.properties";
	
	private final File configFile;
	private final SimpleCrypto simpleCrypto;
	
	public static class Builder {
		
		private File baseFile;
		
		public Builder(File baseFile) {
			this.baseFile = baseFile;
		}
		
		public ConfigManager buildConfig() {
			return new ConfigManager(new File(baseFile.getAbsolutePath() + File.separator
				+ CONFIG_FILE));
		}
		
		public ConfigManager buildSecret() {
			return new ConfigManager(new File(baseFile.getAbsolutePath() + File.separator
					+ PASSWORD_FILE));
		}
		
	}

	private ConfigManager(File file) {
		configFile = file;
		simpleCrypto = new SimpleCrypto();
	}

	public ConfigPropertiesDTO getConfigPropertiesDTO() throws Exception {
		if (configFile == null) {
			throw new NullPointerException("Config File path is NULL");
		}
 		if (!configFile.exists() || !configFile.canRead()) {
			throw new IOException("Failed to load config.properties : "
					+ configFile.getAbsolutePath());
		}

		FileInputStream fis = new FileInputStream(configFile);
		ConfigPropertiesDTO propDTO;
		Properties prop = new Properties();
		boolean isPlainText = false;
		String cryptAppKey = null;
		String plainAppKey = "";

		try {
			prop.load(fis);

			String routerURL = prop.getProperty(ROUTER_URL);
			String tenantName = prop.getProperty(TENANT_NAME, "DUMMYTENANT");
			String appID = prop.getProperty(APP_ID);
			String appKey = prop.getProperty(APP_KEY);

			if (routerURL == null || routerURL.isEmpty())
				throw new Exception("Failed to load ROUTER_URL or the property is empty");
			if (tenantName == null || tenantName.isEmpty())
				throw new Exception("Failed to load TENANT_NAME or the property is empty");
			if (appID == null || appID.isEmpty())
				throw new Exception("Failed to load APP_ID or the property is empty");
			if (appKey == null || appKey.isEmpty())
				throw new Exception("Failed to load APP_KEY or the property is empty");

			if (appKey.contains(PREFIX_CRYPT)) {
				plainAppKey = appKey.substring(appKey.indexOf(":") + 1);

				plainAppKey = simpleCrypto.decrypt(plainAppKey);
				cryptAppKey = appKey;
			} else {
				plainAppKey = appKey;

				isPlainText = true;
				cryptAppKey = PREFIX_CRYPT + simpleCrypto.encrypt(plainAppKey);
			}
			
			propDTO = new ConfigPropertiesDTO(routerURL, tenantName,
					Integer.parseInt(appID), plainAppKey);
		} catch (Exception ex) {
			throw ex;
		} finally {
			fis.close();
		}

		if (isPlainText) {
			FileOutputStream fos = new FileOutputStream(configFile);
			prop.setProperty(APP_KEY, cryptAppKey);

			try {
				prop.store(fos, null);
			} catch (IOException io) {
				throw io;
			} finally {
				fos.close();
			}
		}

		return propDTO;
	}
	
	public SecretPropertiesDTO getSecretPropertiesDTO() throws Exception {
		if (configFile == null) {
			throw new NullPointerException("Secret File path is NULL");
		}
		if (!configFile.exists() || !configFile.canRead()) {
			throw new Exception("Failed to load secret.properties : "
					+ configFile.getAbsolutePath());
		}

		FileInputStream fis = new FileInputStream(configFile);
		SecretPropertiesDTO propDTO;
		Properties prop = new Properties();
		boolean isPlainText = false;
		String cryptAppKey = null;
		String plainAppKey = "";

		try {
			prop.load(fis);

			String secret = prop.getProperty(SECRET);

			if (secret == null || secret.isEmpty())
				throw new Exception("Failed to load SECRET or the property is empty");

			if (secret.contains(PREFIX_CRYPT)) {
				plainAppKey = secret.substring(secret.indexOf(":") + 1);
				plainAppKey = simpleCrypto.decrypt(plainAppKey);
				
				cryptAppKey = secret;
			} else {
				plainAppKey = secret;

				isPlainText = true;
				cryptAppKey = PREFIX_CRYPT + simpleCrypto.encrypt(secret);
			}
			
			propDTO = new SecretPropertiesDTO(plainAppKey);
		} catch (Exception ex) {
			throw ex;
		} finally {
			fis.close();
		}

		if (isPlainText) {
			FileOutputStream fos = new FileOutputStream(configFile);
			prop.setProperty(SECRET, cryptAppKey);

			try {
				prop.store(fos, null);
			} catch (IOException io) {
				throw io;
			} finally {
				fos.close();
			}
		}

		return propDTO;
	}

}
