/*
 * Created on November 9, 2017
 * Updated on February 25, 2019: Supports SkyDRM
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.configuration;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Properties;

import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import jakarta.xml.bind.DatatypeConverter;

public class NXLPropertiesFile implements NextLabsConfigInterface {

	private Properties configProp;
	private static final String ENV_DCE_ROOT = "RMX_ROOT";
	private static final String PATH_CONFIG_FILE = "\\config\\config.properties";
	
	/** Cryptography configuration */
	/** Interim solution before RMC implements partial decryption
	 * 		or RMJavaSDK implements policy evaluation
	 */
	private static final String KEY = "DCE1234!NeXTlAbS"; 			// 128 bit key
	private static final SecureRandom RANDOM = new SecureRandom();
	private static final String ALGORITHM = "AES";
	private static final String CHAR_SET = "UTF-8";
	private static final String TRANSFORMATION = "AES/CBC/PKCS5PADDING";
	private static final String PREFIX_CRYPT = "crypt:";
	
	/** SkyDRM details */
	private static final String ROUTER_URL = "ROUTER_URL";
	private static final String TENANT_NAME = "TENANT_NAME";
	private static final String APP_ID = "APP_ID";
	private static final String APP_KEY = "APP_KEY";
	
	private static final String DEFAULT_ROUTER_URL = "";
	private static final String DEFAULT_TENANT_NAME = "";
	private static final String DEFAULT_APP_ID = "";
	private static final String DEFAULT_APP_KEY = "";
	
	
	/**
	 * Constructor.
	 */
	public NXLPropertiesFile() {
		configProp = new Properties();
		
		try {
			String dce_path = System.getenv(ENV_DCE_ROOT);
			File configFile = new File(dce_path + PATH_CONFIG_FILE);
						
			if (configFile.exists()) {
				InputStream inputFile = new FileInputStream(configFile);
				
				configProp.load(inputFile);
			}
		} catch (Exception ex) {
			System.out.println(ex.getMessage());
			ex.printStackTrace(System.out);
		}
	}
	
	/**
	 * Returns the router URL of SkyDRM from the configuration.
	 * @return (String) The router URL of SkyDRM.
	 */
	public String getRouterURL() {
		if (configProp != null && configProp.containsKey(ROUTER_URL)) {
			return configProp.getProperty(
					ROUTER_URL, DEFAULT_ROUTER_URL);
		}
		
		// Else case
		return null;
	}
	
	/**
	 * Returns the tenant name of SkyDRM from the configuration.
	 * @return (String) The tenant name of SkyDRM for DCE.
	 */
	public String getTenantName() {
		if (configProp != null && configProp.containsKey(TENANT_NAME)) {
			return configProp.getProperty(
					TENANT_NAME, DEFAULT_TENANT_NAME);
		}
		
		// Else case
		return null;
	}
	
	/**
	 * Returns the App ID of SkyDRM from the configuration.
	 * @return (int) The APP ID of SkyDRM for DCE.
	 */
	public int getAppID() {
		if (configProp != null && configProp.containsKey(APP_ID)) {
			try {
				return Integer.parseInt(configProp.getProperty(
						APP_ID, DEFAULT_APP_ID));
			} catch (Exception ex) {
				System.out.println(ex.getMessage());
				ex.printStackTrace(System.out);
			}
			
		}
		
		// Else or exceptional case
		return -1;
	}
	
	/**
	 * Returns the App Key of SkyDRM from the configuration.
	 * @return (String) The App Key of SkyDRM for DCE.
	 */
	public String getAppKey() {
		if (configProp != null && configProp.containsKey(APP_KEY)) {
			String ciphertext = configProp.getProperty(
					APP_KEY, DEFAULT_APP_KEY);
			
			try {
				if (ciphertext.contains(PREFIX_CRYPT))
					ciphertext = ciphertext.substring(ciphertext.indexOf(':') + 1);
				
				return decrypt(ciphertext);
			} catch (Exception ex) {
				System.out.println(ex.getMessage());
				ex.printStackTrace(System.out);
			}
		}
		
		// Else of exceptional case
		return null;
	}
	    
    /**
	 * Returns whether the config interface has data loaded or not.
	 * @return (boolean) <code>true</code> if the data has been loaded.
	 */
	public boolean hasData() {
		return !configProp.isEmpty();
	}
	
	/**
	 * 
	 * @param plaintext
	 * @return (String) The ciphertext. 
	 * @throws Exception
	 */
	private String encrypt(String plaintext) throws Exception {
		// 16 bytes random IV
		byte[] bytesIV = new byte[16];
		RANDOM.nextBytes(bytesIV);
		
		IvParameterSpec iv = new IvParameterSpec(bytesIV);
        SecretKeySpec skeySpec = new SecretKeySpec(KEY.getBytes(CHAR_SET), ALGORITHM);

        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        cipher.init(Cipher.ENCRYPT_MODE, skeySpec, iv);

        byte[] encrypted = cipher.doFinal(plaintext.getBytes());
        byte[] ivencrypted = new byte[bytesIV.length + encrypted.length];
        System.arraycopy(bytesIV, 0, ivencrypted, 0, bytesIV.length);
        System.arraycopy(encrypted, 0, ivencrypted, bytesIV.length, encrypted.length);

        return DatatypeConverter.printBase64Binary(ivencrypted);
    }

	/**
	 * 
	 * @param ciphertext
	 * @return (String) The plaintext.
	 * @throws Exception
	 */
    private String decrypt(String ciphertext) throws Exception {
    	byte[] ivencrypted = DatatypeConverter.parseBase64Binary(ciphertext);
    	// 16 bytes IV
		byte[] bytesIV = Arrays.copyOfRange(ivencrypted, 0, 16);
		
        //IvParameterSpec iv = new IvParameterSpec(INITVECTOR.getBytes(CHAR_SET));
		IvParameterSpec iv = new IvParameterSpec(bytesIV);
        SecretKeySpec skeySpec = new SecretKeySpec(KEY.getBytes(CHAR_SET), ALGORITHM);

        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        cipher.init(Cipher.DECRYPT_MODE, skeySpec, iv);

        byte[] original = cipher.doFinal(
        		Arrays.copyOfRange(ivencrypted, 16, ivencrypted.length));

        return new String(original);
    }

}
