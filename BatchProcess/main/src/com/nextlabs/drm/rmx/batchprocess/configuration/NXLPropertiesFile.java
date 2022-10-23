package com.nextlabs.drm.rmx.batchprocess.configuration;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Properties;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class NXLPropertiesFile {
	
	private static final Logger LOGGER = LogManager.getLogger("BPLOGGER");

	// File name for connection properties
	public static final String PROFILE_PROPERTIES_FILE = "profile.properties";
	public static final String SKYDRMPROFILE_PROPERTIES_FILE = "skydrmprofile.properties";
	
	/** Tc SOA connection's key name */
	private static final String TC_HOST = "TC_HOST";
	private static final String TC_USER_NAME = "TC_USER_NAME";
	private static final String TC_USER_PASSWORD = "TC_USER_PASSWORD";
	private static final String NUM_OF_THREADS = "NUM_OF_THREADS";
	private static final String STAGING_FOLDER = "STAGING_FOLDER";
	
	public static final int MAX_THREADS = 16;
	
	/** Tc SOA connections' default values */
	private static final String DEFAULT_TC_HOST = "";
	private static final String DEFAULT_TC_USER_NAME = "";
	private static final String DEFAULT_TC_USER_PASSWORD = "";
	private static final String DEFAULT_NUM_OF_THREADS = "0";
	private static final String DEFAULT_STAGING_FOLDER = "";
	
	/** Policy controller details' key name */
	private static final String KEY_STORE_NAME = "KEY_STORE_NAME";
	private static final String KEY_STORE_PASSWORD = "KEY_STORE_PASSWORD";
	private static final String TRUST_STORE_NAME = "TRUST_STORE_NAME";
	private static final String TRUST_STORE_PASSWORD = "TRUST_STORE_PASSWORD";
	private static final String PC_HOST_NAME = "PC_HOST_NAME";
	private static final String RMI_PORT_NUM = "RMI_PORT_NUM";
	private static final String PREFIX_CRYPT = "crypt:";
	
	/** Policy controller details' default values */
	private static final String DEFAULT_KEY_STORE_NAME = "";
	private static final String DEFAULT_KEY_STORE_PASSWORD = "";
	private static final String DEFAULT_TRUST_STORE_NAME = "";
	private static final String DEFAULT_TRUST_STORE_PASSWORD = "";
	private static final String DEFAULT_PC_HOST_NAME = "";
	private static final String DEFAULT_RMI_PORT_NUM = "1099";
	
	/** SkyDRM details' key name */
	private static final String ROUTER_URL = "ROUTER_URL";
	private static final String TENANT_NAME = "TENANT_NAME";
	private static final String APP_ID = "APP_ID";
	private static final String APP_KEY = "APP_KEY";
	
	/** SkyDRM details' default values */
	private static final String DEFAULT_ROUTER_URL = "";
	private static final String DEFAULT_TENANT_NAME = "DUMMYTENANT";
	private static final String DEFAULT_APP_ID = "0";
	private static final String DEFAULT_APP_KEY = "";
	
	private final Properties profileProp;
	private final SimpleCrypto simpleCrypto;
	private boolean hasPlaintextData;
		
	/**
	 * constructor
	 */
	public NXLPropertiesFile(String propertiesFile) throws Exception {
		profileProp = new Properties();
		simpleCrypto = new SimpleCrypto();
		hasPlaintextData = false;
		
		File profileFile = new File(propertiesFile);
					
		if (profileFile.exists()) {
			try (InputStream inputFile = new FileInputStream(profileFile)) {
				
				profileProp.load(inputFile);
				validateConnectionProperties();
				
				testPlaintextData();
				if (hasPlaintextData) {
					try (OutputStream outputFile = new FileOutputStream(profileFile)) {
						profileProp.store(outputFile, "");
					}
				}
			}
		}
	}
	
	public String getTcHost() {
		if (profileProp != null && profileProp.containsKey(TC_HOST))
			return profileProp.getProperty(TC_HOST, DEFAULT_TC_HOST);
		
		// else case
		return null;
	}
	
	public String getTcUserName() {
		if (profileProp != null && profileProp.containsKey(TC_USER_NAME))
			return profileProp.getProperty(TC_USER_NAME, DEFAULT_TC_USER_NAME);
		
		// else case
		return null;
	}
	
	public String getTcUserPassword() {
		if (profileProp != null && profileProp.containsKey(TC_USER_PASSWORD)) {
			String ciphertext = profileProp.getProperty(
					TC_USER_PASSWORD, DEFAULT_TC_USER_PASSWORD);
			    		
			try {
				if (ciphertext.contains(PREFIX_CRYPT)) {
					ciphertext = ciphertext.substring(ciphertext.indexOf(":") + 1);
					return simpleCrypto.decrypt(ciphertext);
				} else {
					hasPlaintextData = true;
					profileProp.put(TC_USER_PASSWORD, PREFIX_CRYPT + simpleCrypto.encrypt(ciphertext));
					return ciphertext;
				}
			} catch (Exception ex) {
				LOGGER.error(ex.getMessage(), ex);
			}
		}
		
		// else or exceptional case
		return null;
	}
	
	public int getNumOfThreads() {
		int numThreads = 0;
		if (profileProp != null && profileProp.containsKey(NUM_OF_THREADS))
			numThreads = Integer.parseInt(profileProp.getProperty(NUM_OF_THREADS, DEFAULT_NUM_OF_THREADS));
		
		if (numThreads < 0 || numThreads > MAX_THREADS) {
			numThreads = Integer.parseInt(DEFAULT_NUM_OF_THREADS);
			LOGGER.warn("Warning: invalid NUM_OF_THREADS (0 < n < {}), set to default {}", 
					MAX_THREADS, numThreads);
		}
		
		return numThreads;
	}
	
	public String getStagingFolder() {
		if (profileProp != null && profileProp.containsKey(STAGING_FOLDER))
			return profileProp.getProperty(STAGING_FOLDER, DEFAULT_STAGING_FOLDER);
		
		// else case
		return DEFAULT_STAGING_FOLDER;
	}
	
	// RMS JAVA SDK
	/**
	 * Returns the absolute path & file name of the key store from the configuration.
	 * @return (String) The path & file name of the key store.
	 */
	public String getKeyStoreName() {
		if (profileProp != null && profileProp.containsKey(KEY_STORE_NAME)) {
			return profileProp.getProperty(
					KEY_STORE_NAME, DEFAULT_KEY_STORE_NAME);
		}

		// else case
		return null;
	}
	
	/**
	 * Returns the password for the key store.
	 * @return (String) The password for the key store.
	 */
	public String getKeyStorePassword() {
		if (profileProp != null && profileProp.containsKey(KEY_STORE_PASSWORD)) {
			String ciphertext = profileProp.getProperty(
					KEY_STORE_PASSWORD, DEFAULT_KEY_STORE_PASSWORD);
			    		
			try {
				if (ciphertext.contains(PREFIX_CRYPT)) {
					ciphertext = ciphertext.substring(ciphertext.indexOf(":") + 1);
					return simpleCrypto.decrypt(ciphertext);
				} else {
					hasPlaintextData = true;
					profileProp.put(KEY_STORE_PASSWORD, PREFIX_CRYPT + simpleCrypto.encrypt(ciphertext));
					return ciphertext;
				}
			} catch (Exception ex) {
				LOGGER.error(ex.getMessage(), ex);
			}
		}

		// else or exceptional case
		return null;
	}
	
	/**
	 * Returns the absolute path & file name of the trust store from the configuration.
	 * @return (String) The path & file name of the trust store.
	 */
	public String getTrustStoreName() {
		if (profileProp != null && profileProp.containsKey(TRUST_STORE_NAME)) {
			return profileProp.getProperty(
					TRUST_STORE_NAME, DEFAULT_TRUST_STORE_NAME);
		}
		
		// else case
		return null;
	}
	
	/**
	 * Returns the password for the trust store.
	 * @return (String) The password for the trust store.
	 */
	public String getTrustStorePassword() {
		if (profileProp != null && profileProp.containsKey(TRUST_STORE_PASSWORD)) {
			String ciphertext = profileProp.getProperty(
					TRUST_STORE_PASSWORD, DEFAULT_TRUST_STORE_PASSWORD);
			
			try {
				if (ciphertext.contains(PREFIX_CRYPT)) {
					ciphertext = ciphertext.substring(ciphertext.indexOf(":") + 1);
					return simpleCrypto.decrypt(ciphertext);
				} else {
					hasPlaintextData = true;
					profileProp.put(TRUST_STORE_PASSWORD, PREFIX_CRYPT + simpleCrypto.encrypt(ciphertext));
					return ciphertext;
				}
			} catch (Exception ex) {
				LOGGER.error(ex.getMessage(), ex);
			}
		}
		
		// else or exceptional case
		return null;
	}
	
	/**
	 * Returns the host name of the Policy Controller that provides the 
	 * 		Key Management Service. 
	 * @return (String) The host name of the Policy Controller. 
	 */
	public String getPcHostName() {
		if (profileProp != null && profileProp.containsKey(PC_HOST_NAME)) {
			return profileProp.getProperty(PC_HOST_NAME, DEFAULT_PC_HOST_NAME);
		}
		
		// else case
		return null;
	}
	
	/**
	 * Returns the RMI port number of the Policy Controller that provides 
	 * 		the Key Management Service.
	 * @return (int) The RMI port number. 
	 */
	public int getRmiPortNum() {
		if (profileProp != null && profileProp.containsKey(RMI_PORT_NUM)) {
			return Integer.parseInt(
					profileProp.getProperty(RMI_PORT_NUM, DEFAULT_RMI_PORT_NUM));
		}
		
		// else case
		return 0;
	}
	    
    /**
	 * Returns whether the config interface has data loaded or not.
	 * @return (boolean) <code>true</code> if the data has been loaded.
	 */
	public boolean hasData() {
		return !profileProp.isEmpty();
	}
	
	/**
	 * validate the connection properties
	 */
	private void validateConnectionProperties() throws Exception {
		String tcHost = getTcHost();
		String tcUserName = getTcUserName();
		String tcUserPassword = getTcUserPassword();
		
		if (tcHost == null || tcHost.trim().length() <= 0) {
			throw new Exception("Invalid " + TC_HOST);
		}
		
		if (tcUserName == null || tcUserName.trim().length() <= 0) {
			throw new Exception("Invalid " + TC_USER_NAME);
		}
		
		if (tcUserPassword == null || tcUserPassword.trim().length() <= 0) {
			throw new Exception("Invalid " + TC_USER_PASSWORD);
		}
	}
	
	private void testPlaintextData() {
		getTcUserPassword();
		getKeyStorePassword();
		getTrustStorePassword();
		getAppKey();
	}
	
	public String getRouterURL() {
		if (profileProp != null && profileProp.containsKey(ROUTER_URL)) {
			return profileProp.getProperty(ROUTER_URL, DEFAULT_ROUTER_URL);
		}
		
		return null;
	}
	
	public String getTenantName() {
		if (profileProp != null && profileProp.containsKey(TENANT_NAME)) {
			return profileProp.getProperty(TENANT_NAME, DEFAULT_TENANT_NAME);
		}
		
		return null;
	}
	
	public int getAppID() {		
		if (profileProp != null && profileProp.containsKey(APP_ID)) {
			return Integer.parseInt(
					profileProp.getProperty(APP_ID, DEFAULT_APP_ID));
		}
		
		// else case
		return 0;
	}
	
	public String getAppKey() {
		if (profileProp != null && profileProp.containsKey(APP_KEY)) {
			String ciphertext = profileProp.getProperty(
					APP_KEY, DEFAULT_APP_KEY);
			
			try {
				if (ciphertext.contains(PREFIX_CRYPT)) {
					ciphertext = ciphertext.substring(ciphertext.indexOf(":") + 1);
					return simpleCrypto.decrypt(ciphertext);
				} else {
					hasPlaintextData = true;
					profileProp.put(APP_KEY, PREFIX_CRYPT + simpleCrypto.encrypt(ciphertext));
					return ciphertext;
				}
			} catch (Exception ex) {
				LOGGER.error(ex.getMessage(), ex);
			}
		}
		
		// else or exceptional case
		return null;
	}
	
	
}
