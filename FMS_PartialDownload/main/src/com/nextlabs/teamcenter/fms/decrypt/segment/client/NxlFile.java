/*
 * Created on November 19, 2019
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.decrypt.segment.client;

import java.util.Arrays;

public class NxlFile {
	
	private final long contentLength;
	private final byte[] header;
	
	public NxlFile(byte[] header, long contentLength) {
		this.contentLength = contentLength;
		this.header = Arrays.copyOf(header, header.length);
	}
	
	public long getContentLength() {
		return contentLength;
	}
	
	public byte[] getHeader() {
		return header;
	}

}
