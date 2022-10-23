package com.nextlabs.drm.rmx.test;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.junit.runner.JUnitCore;
import org.junit.runner.Result;
import org.junit.runner.notification.Failure;
//import org.slf4j.Logger;
//import org.slf4j.LoggerFactory;


/**
 * 
 * @author clow
 * Test runner
 *
 */

public class TestRunner {
	
	//private static final Logger LOGGER = LoggerFactory.getLogger("TRANSLATOR_TEST_LOGGER");
	private static final Logger LOGGER = LogManager.getLogger("TRANSLATOR_TEST_LOGGER");
	
	public static void main(String[] args) {
		Result result = JUnitCore.runClasses(JUnitTestSuite.class);
		
		for (Failure failure : result.getFailures()) {
			if (failure != null) System.out.println(failure.toString());
		}
		
		LOGGER.info("");
		LOGGER.info("=============		TEST RESULT		=============");
		LOGGER.info("Test runtime: " + result.getRunTime() + " ms");
		LOGGER.info("Test cases [executed]: " + result.getRunCount());
		LOGGER.info("Test cases [failed]: " + result.getFailureCount());
		LOGGER.info("Test cases [passed]: " 
				+ (result.getRunCount() - result.getFailureCount()));
		
		System.out.println("=============		TEST RESULT		=============");
		System.out.println("Test runtime: " + result.getRunTime() + " ms");
		System.out.println("Test cases [executed]: " + result.getRunCount());
		System.out.println("Test cases [failed]: " + result.getFailureCount());
		System.out.println("Test cases [passed]: " 
				+ (result.getRunCount() - result.getFailureCount()));
		
		System.exit(0);
	}

}
