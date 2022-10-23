package com.nextlabs.drm.rmx.test;

import static org.junit.Assert.assertEquals;

import java.io.File;
import java.io.FileOutputStream;
import java.util.Properties;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
//import org.slf4j.Logger;
//import org.slf4j.LoggerFactory;

import com.nextlabs.drm.rmx.configuration.ConfigManager;
import com.nextlabs.drm.rmx.configuration.ConfigPropertiesDTO;

public class ConfigManagerTest {
	
	//private final static Logger LOGGER = LoggerFactory.getLogger("TRANSLATOR_TEST_LOGGER");
	private final static Logger LOGGER = LogManager.getLogger("TRANSLATOR_TEST_LOGGER");
	private static File baseFile;
	private static ConfigManager configManager;
	private static final String CONFIG_FILE = "config.properties";
	
	private static final String ROUTER_URL = "ROUTER_URL";
	private static final String TENANT_NAME = "TENANT_NAME";
	private static final String APP_ID = "APP_ID";
	private static final String APP_KEY = "APP_KEY";
	
	private static final String ROUTER_URL_VAL = "https://10.23.58.178:8443/router";
	private static final String TENANT_NAME_VAL = "t-e7d228c15b5e48a09f41538f09ce8df0";
	private static final int APP_ID_VAL = 5;
	private static final String APP_KEY_VAL = "CE78FAC91597260CF184E0BFD6CBE66F";
	
	
	
	@BeforeClass
	public static void init() throws Exception {
		baseFile = new File(ClassLoader.getSystemClassLoader().getResource(".").toURI());
		File configFile = new File(baseFile.getAbsolutePath() 
				+ File.separator + CONFIG_FILE);
		
		if (!configFile.exists()) configFile.createNewFile();
		
		FileOutputStream fos = new FileOutputStream(configFile);
		Properties prop = new Properties();
		
		prop.put(ROUTER_URL, ROUTER_URL_VAL);
		prop.put(TENANT_NAME, TENANT_NAME_VAL);
		prop.put(APP_ID, String.valueOf(APP_ID_VAL));
		prop.put(APP_KEY, APP_KEY_VAL);
		
		prop.store(fos, null);
		fos.close();
		
		configManager = new ConfigManager.Builder(baseFile).buildConfig();
	}
	
	@AfterClass
	public static void teardown() throws Exception {
		baseFile = new File(ClassLoader.getSystemClassLoader().getResource(".").toURI());
		File configFile = new File(baseFile.getAbsolutePath() 
				+ File.separator + CONFIG_FILE);
		
		if (configFile.exists()) configFile.delete();
	}
	
	@BeforeClass
	public static void initialize() throws Exception {
		
	}
	
	@Test
	public void getConfigPropertiesDTO1() throws Exception {
		ConfigPropertiesDTO configDTO = configManager.getConfigPropertiesDTO();
		
		// Test result
		LOGGER.info("1. Start [" + this.getClass().getSimpleName() + "] => getConfigPropertiesDTO1");
		LOGGER.info("	rounter_url: " + configDTO.getRouterURL());
		LOGGER.info("	tenant_name: " + configDTO.getTenantName());
		LOGGER.info("	app_id: " + configDTO.getAppID());
		LOGGER.info("	app_key: " + configDTO.getAppKey());
		LOGGER.info("1. End => Assert equal");
		LOGGER.info("");
		LOGGER.info("");
		
		assertEquals(configDTO.getRouterURL(), ROUTER_URL_VAL);
		assertEquals(configDTO.getTenantName(), TENANT_NAME_VAL);
		assertEquals(configDTO.getAppID(), APP_ID_VAL);
		assertEquals(configDTO.getAppKey(), APP_KEY_VAL);
	}

}
