/*
 * Created on July 7, 2020
 *
 * All sources, binaries and HTML pages (C) copyright 2020 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.racrmx;

public class RACRMXInstance {
	
	public native boolean loadDRmUser();
	
	public native boolean loadDRmInstance();
	
	public native boolean isFileProtected(String filepath);
	
	public native long setViewOverlay(long hwnd);
	
	public native long renderOverlay(String filepath);
	
	public native long clearViewOverlay();
	
	private static RACRMXInstance instance;
	
	static {
		 System.loadLibrary("RACRMX");
	}
	
	private RACRMXInstance() throws Exception {
		if (!loadDRmUser())
			throw new Exception("Failed to loadDRmUser");
		
		if (!loadDRmInstance())
			throw new Exception("Failed to loadDRmInstance");
	}
	
	public synchronized static RACRMXInstance getInstance() throws Exception {
		if (instance == null) 
			instance = new RACRMXInstance();
		
		return instance;
	}

}
