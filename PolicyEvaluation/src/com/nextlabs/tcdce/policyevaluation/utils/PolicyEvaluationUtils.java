package com.nextlabs.tcdce.policyevaluation.utils;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

public class PolicyEvaluationUtils {

	/**
	 * load the properties from file
	 */
	public static Properties loadPropertiesFromFile(String propFilePath) throws IOException {
		Properties props = new Properties();
		InputStream input = new FileInputStream(propFilePath);
		try {
			props.load(input);
		} catch (IOException e) {
			System.out.println("PolicyEvaluationUtils.loadPropertiesFromFile() caught exception: " + e.getMessage());
			e.printStackTrace();
			throw e;
		} finally {
			if (input != null) {
				input.close();
			}
		}
		return props;
	}
}
