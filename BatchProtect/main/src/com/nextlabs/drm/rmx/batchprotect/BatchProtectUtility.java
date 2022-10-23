package com.nextlabs.drm.rmx.batchprotect;

/**
 * Created on December 2, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Properties;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import static com.nextlabs.drm.rmx.batchprotect.configuration.BatchProtectConstants.*;
import com.teamcenter.soa.client.model.ErrorStack;
import com.teamcenter.soa.client.model.ServiceData;

public class BatchProtectUtility {

	private final static Logger logger = LogManager.getLogger("BPLOGGER");
	
	/**
	 * log the service data returned from TC service
	 */
	public static void logServiceData(ServiceData serviceData) {
		logger.info("    Size of created objects: {}", serviceData.sizeOfCreatedObjects());
		logger.info("    Size of partial errors: {}", serviceData.sizeOfPartialErrors());
		logger.info("    Size of plain objects: {}", serviceData.sizeOfPlainObjects());
		
		for (int i = 0; i < serviceData.sizeOfPartialErrors(); i++) {
			ErrorStack errStack = serviceData.getPartialError(i);
			
			for (String errMsg : errStack.getMessages()) {
				logger.info("      Error message: {}", errMsg);
			}
		}
	}

	/**
	 * load the properties from file
	 */
	public static Properties loadPropertiesFromFile(String propFilePath) throws IOException {
		Properties props = new Properties();
		InputStream input = null;
		
		try {
			input = new FileInputStream(propFilePath);
			
			props.load(input);
		} catch (IOException e) {
			logger.error("BatchProtect.loadPropertiesFromFile() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
			
			throw e;
		} finally {
			if (input != null) {
				try {
					input.close();
				} catch (IOException e) {
					logger.error("BatchProtect.loadPropertiesFromFile() caught exception: {}", e.getMessage());
					logger.error("Stack trace: ", e);
				}
			}
		}
		
		return props;
	}

	/**
	 * write the properties to file
	 */
	public static void writePropertiesToFile(String propFilePath, Properties props) {
		OutputStream output = null;
		
		try {
			output = new FileOutputStream(propFilePath);
			
			props.store(output, "");
		} catch (IOException e) {
			logger.error("BatchProtect.writePropertiesToFile() caught exception: {}", e.getMessage());
			logger.error("Stack trace: ", e);
		} finally {
			if (output != null) {
				try {
					output.close();
				} catch (IOException e) {
					logger.error("BatchProtect.writePropertiesToFile() caught exception: {}", e.getMessage());
					logger.error("Stack trace: ", e);
				}
			}
		}
	}

	/**
	 * validate the connection properties
	 */
	public static void validateConnectionProperties(Properties connProps) throws Exception {
		String tc_soa_host = connProps.getProperty(TC_SOA_HOST);
		String tc_user_name = connProps.getProperty(TC_USER_NAME);
		String tc_user_password = connProps.getProperty(TC_USER_PASSWORD);
		
		if (tc_soa_host == null || tc_soa_host.trim().length() <= 0) {
			throw new Exception("Invalid " + TC_SOA_HOST);
		}
		
		if (tc_user_name == null || tc_user_name.trim().length() <= 0) {
			throw new Exception("Invalid " + TC_USER_NAME);
		}
		
		if (tc_user_password == null || tc_user_password.trim().length() <= 0) {
			throw new Exception("Invalid " + TC_USER_PASSWORD);
		}
	}
	
}
