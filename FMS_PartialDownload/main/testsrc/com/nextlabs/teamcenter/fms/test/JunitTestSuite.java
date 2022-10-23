/*
 * Created on November 15, 2017
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.test;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;

/**
 * 
 * @author clow
 * Test suite for NextLabs partial download support
 *
 */

// JUnit Test Suite
@RunWith(Suite.class)

@Suite.SuiteClasses({
	//com.nextlabs.teamcenter.fms.test.server.TextFileTest.class,
	//com.nextlabs.teamcenter.fms.test.server.JtFileTest.class,
	//com.nextlabs.teamcenter.fms.test.server.PrtFileTest.class,
	com.nextlabs.teamcenter.fms.test.client.TextFileTest.class,
	com.nextlabs.teamcenter.fms.test.client.JtFileTest.class,
	com.nextlabs.teamcenter.fms.test.client.PrtFileTest.class
})

public class JunitTestSuite {

}
