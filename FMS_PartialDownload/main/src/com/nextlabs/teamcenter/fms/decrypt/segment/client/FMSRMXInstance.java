/*
 * Created on November 19, 2019
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.decrypt.segment.client;

public class FMSRMXInstance {
	
	public native boolean loadDRmUser();
		
	public native boolean buildNxlHeader(byte[] header, long[] outputData);
	
	public native long decryptPartial(byte[] input, byte[] output, byte[] header, long offset, long length);
			
	static {
		System.loadLibrary("FMSRMX");
	}
	
	public FMSRMXInstance() throws Exception {
		if (!loadDRmUser())
			throw new Exception("Failed to loadDRmUser");
	}
	
}
