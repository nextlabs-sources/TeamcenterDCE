/*
 * Created on March 22, 2018
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.configuration;

import com.nextlabs.nxl.NxlFile;

public abstract class NextLabsConstants {

	/**
	 * The block size of NextLabs encrypted file, the minimum unit for
	 * decryption.
	 */
	public static final int NXL_BLOCK_SIZE = 512;

	/** The NXL block multiplier for BB */
	public static final int BLOCK_MULTIPLIER = 32;

	/** The length of a NextLabs encryption header. */
	public static final int HEADER_LENGTH = NxlFile.COMPLETE_HEADER_SIZE;

	/**
	 * The NextLabs format which is defined in the first few bytes of the file
	 * header.
	 */
	public static final String NXL_HEADER_FORMAT = "NXLFMT";
	
	/**
	 * The version number and build number
	 */
	public static final String NXL_BUILD_VERSION =  " (NextLabs) 1.0.1";
	
}
