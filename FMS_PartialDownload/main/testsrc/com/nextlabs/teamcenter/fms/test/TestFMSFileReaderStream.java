/*
 * Created on November 15, 2017
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.test;

import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import jakarta.xml.bind.DatatypeConverter;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.teamcenter.fms.decrypt.segment.InterfaceBufferHandler;
import com.teamcenter.fms.decrypt.segment.InterfaceFMSFileReaderStream;

/**
 * 
 * @author clow
 * Mock up class to simulate the input as InterfaceFMSFileReaderStream
 *
 */

public class TestFMSFileReaderStream extends BufferedInputStream 
		implements InterfaceFMSFileReaderStream {
	
	private final static Logger LOGGER = LogManager.getLogger("PARTIALDW_TEST_LOGGER");
	
	long fileSize;
		
	public TestFMSFileReaderStream(FileInputStream fis) throws IOException {
		super(fis);
		
		fileSize = fis.getChannel().size();
	}

	@Override
	public long fileSize() throws Exception {
		return fileSize;
	}
	
	@Override
	public int read(final long fileOffset, ByteBuffer bb, 
			final InterfaceBufferHandler buffHandler, final int len) throws Exception {
		super.mark((int) fileOffset + len);
		
		long totalSkipped = super.skip(fileOffset);
		while (totalSkipped < fileOffset) {
			totalSkipped += super.skip(fileOffset - totalSkipped);
			
			if (totalSkipped <= 0)
				break;
		}
		LOGGER.debug("totalSkipped: {}, fileOffset: {}", totalSkipped, fileOffset);
		
		int byteRead = 0;
		byte[] buffer = new byte[len];
		byteRead = super.read(buffer, 0, len);
		LOGGER.debug("byteRead: {}", byteRead);
		LOGGER.trace("rawdata:{}{}", System.lineSeparator(), DatatypeConverter.printHexBinary(buffer));
		
		int byteIndex = 0;
		while (bb.hasRemaining()) {
			for ( ; byteIndex < buffer.length && bb.hasRemaining() && byteIndex < byteRead; byteIndex++) {
				byte plainData = buffer[byteIndex];
				
				//LOGGER.debug("plainData put: " + plainData);
				bb.put(plainData);
			}
			
			int size = bb.position();
			LOGGER.debug("flushing bb ... {}", size);
			bb.flip();
			buffHandler.onBuffer(bb, 0, size);
			
			if (byteIndex == buffer.length || byteIndex == byteRead || byteRead <= -1) {
				break;
			}
			
			while (!bb.hasRemaining() && byteIndex < buffer.length && byteIndex < byteRead) {
				Thread.sleep(100);
				LOGGER.debug("waiting for bb to be processed ...");
			}
		}
		
		super.reset();
		
		LOGGER.debug("return byteRead: {}", byteRead);
		return byteRead;
	}

	// NOT IN USED after interface changed
	public int read(long fileOffset, byte[] b, int off, int len)
			throws Exception {
		super.mark((int) fileOffset + len);
		LOGGER.debug(super.marklimit);
		LOGGER.debug(super.markpos);
		LOGGER.debug(fileOffset);
		
		long totalSkipped = super.skip(fileOffset);
		while (totalSkipped < fileOffset) {
			totalSkipped += super.skip(fileOffset - totalSkipped);
		}
		
		LOGGER.debug("skip: {}", totalSkipped);
		LOGGER.debug("off: {}", off);
		LOGGER.debug("len: {}", len);
		int byteRead = super.read(b, off, len);
		
		super.reset();
		
		return byteRead;
	}

	@Override
	public void close() throws IOException {
		super.close();
	}

}
