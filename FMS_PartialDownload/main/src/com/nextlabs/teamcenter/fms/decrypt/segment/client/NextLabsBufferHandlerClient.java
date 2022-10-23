/*
 * Created on November 19, 2019
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.decrypt.segment.client;

import java.nio.ByteBuffer;
import java.util.Arrays;

import com.nextlabs.teamcenter.fms.decrypt.segment.AbstractNextLabsBufferHandler;
import com.nextlabs.teamcenter.fms.decrypt.segment.BytesRead;
import com.teamcenter.fms.decrypt.segment.InterfaceBufferHandler;

public class NextLabsBufferHandlerClient extends AbstractNextLabsBufferHandler {

	private final BytesRead bytesRead;

	private final NxlFile nxlState;
	private final FMSRMXInstance rightsManager;

	public NextLabsBufferHandlerClient(long fileOffset, int blockSize,
			int blockCount, int len, int bufferEncryptedSize,
			long sourceFileOffset, double startBlock, BytesRead bytesRead,
			NxlFile nxlState, FMSRMXInstance rightsManager,
			ByteBuffer bb, InterfaceBufferHandler buffHandler) {
		super(fileOffset, blockSize, blockCount, len, bufferEncryptedSize, sourceFileOffset, startBlock, bytesRead, bb, buffHandler);
		this.bytesRead = bytesRead;
		
		this.nxlState = nxlState;
		this.rightsManager = rightsManager;
	}
	
	@Override
	protected byte[] processData(byte[] data, final long adjustedFileOffset, final long adjustedLen, 
			final int iBufSize, final int bufBlockCount) {
		bytesRead.setLen(innerLenRead);
		
		byte[] bosPlaintext = new byte[iBufSize];
		byte[] bisCiphertext = Arrays.copyOf(data, iBufSize);
		
		// must handle error case len is zero
		if (LOGGER.isDebugEnabled()) {
			LOGGER.debug("[{}] innerblockCount={} blockCount={} adjustedFileOffset={} adjustedLen={}", 
					uuid, innerBlockCount, blockCount, adjustedFileOffset, adjustedLen);

			LOGGER.debug("[{}] {}, {}, {}, {}, {}, {}", uuid, (rightsManager == null), 
					(bisCiphertext == null), (bosPlaintext == null), (nxlState == null), 
					adjustedFileOffset, adjustedLen);
		}
		
		if (adjustedLen > 0) {
			try {
				long nxl_startTime = System.currentTimeMillis();
				
				//LOGGER.debug("[" + uuid + "] requestCount=" + requestCount.getAndIncrement());
				rightsManager.decryptPartial(bisCiphertext, bosPlaintext, nxlState.getHeader(), adjustedFileOffset, iBufSize);
				//LOGGER.debug("[" + uuid + "] responseCount=" + responseCount.getAndIncrement());
				
				long nxl_endTime = System.currentTimeMillis();
				if (LOGGER.isInfoEnabled()) {
					LOGGER.info("[{}] decrypting {} time-sdk={} ms", 
							uuid, adjustedLen, (nxl_endTime - nxl_startTime));
				}

				innerBlockCount += bufBlockCount;
			} catch (Exception ex) {
				ex.printStackTrace(System.out);
			}
		}

		return Arrays.copyOf(bosPlaintext, (int)adjustedLen);
	}
	
	@Override
	protected long getContentLength() {
		return nxlState.getContentLength();
	}

}
