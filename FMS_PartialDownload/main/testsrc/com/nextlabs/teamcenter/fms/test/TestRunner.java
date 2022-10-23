/*
 * Created on November 15, 2017
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.test;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.junit.runner.JUnitCore;
import org.junit.runner.Result;
import org.junit.runner.notification.Failure;

/**
 * 
 * @author clow
 * Test runner
 *
 */

public class TestRunner {
	
	private final static Logger LOGGER = LogManager.getLogger("PARTIALDW_TEST_LOGGER");
	
	public static void main(String[] args) {
		Result result = JUnitCore.runClasses(JunitTestSuite.class);
		
		for (Failure failure : result.getFailures()) {
			if (failure != null)
				System.out.println(failure.toString());
		}
		
		LOGGER.info("");
		LOGGER.info("=============		TEST RESULT		=============");
		LOGGER.info("Test runtime: {} ms", result.getRunTime());
		LOGGER.info("Test cases [executed]: {}", result.getRunCount());
		LOGGER.info("Test cases [failed]: {}", result.getFailureCount());
		LOGGER.info("Test cases [passed]: {}", 
				(result.getRunCount() - result.getFailureCount()));
		
		System.exit(0);
	}

}
