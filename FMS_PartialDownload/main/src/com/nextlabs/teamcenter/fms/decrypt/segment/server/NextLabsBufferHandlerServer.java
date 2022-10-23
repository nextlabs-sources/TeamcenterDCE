/*
 * Created on November 19, 2019
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.decrypt.segment.server;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;

import com.nextlabs.nxl.NxlFile;
import com.nextlabs.nxl.RightsManager;
import com.nextlabs.teamcenter.fms.decrypt.segment.AbstractNextLabsBufferHandler;
import com.nextlabs.teamcenter.fms.decrypt.segment.BytesRead;
import com.nextlabs.common.shared.Constants.TokenGroupType;
import com.teamcenter.fms.decrypt.segment.InterfaceBufferHandler;

public class NextLabsBufferHandlerServer extends AbstractNextLabsBufferHandler {

	private final NxlFile nxlState;
	private final RightsManager rightsManager;
	
	// SkyDRM's tenant name
	private final String tenantName;
	
	/*
	 * public NextLabsBufferHandler(long fileOffset, int blockSize,
			int blockCount, int len, int bufferEncryptedSize,
			long sourceFileOffset, double startBlock, BytesRead bytesRead,
			ByteBuffer bb, InterfaceBufferHandler buffHandler,
			String tenantName) {
	 * */

	public NextLabsBufferHandlerServer(long fileOffset, int blockSize,
			int blockCount, int len, int bufferEncryptedSize,
			long sourceFileOffset, double startBlock, BytesRead bytesRead,
			NxlFile nxlState, RightsManager rightsManager,
			ByteBuffer bb, InterfaceBufferHandler buffHandler,
			String tenantName) {
		super(fileOffset, blockSize, blockCount, len, bufferEncryptedSize, sourceFileOffset, startBlock, bytesRead, bb, buffHandler);

		this.nxlState = nxlState;
		this.rightsManager = rightsManager;
		
		this.tenantName = tenantName;
	}
	
	@Override
	protected byte[] processData(byte[] data, final long adjustedFileOffset, final long adjustedLen, 
			final int iBufSize, final int bufBlockCount) {
		ByteArrayOutputStream bosPlaintext = new ByteArrayOutputStream();
		ByteArrayInputStream bisCiphertext = new ByteArrayInputStream(data);

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
				rightsManager.decryptPartial(bisCiphertext, bosPlaintext, 
						nxlState, adjustedFileOffset, adjustedLen, null, TokenGroupType.TOKENGROUP_SYSTEMBUCKET);
				//LOGGER.debug("[" + uuid + "] responseCount=" + responseCount.getAndIncrement());
				
				long nxl_endTime = System.currentTimeMillis();
				if (LOGGER.isInfoEnabled()) {
					LOGGER.info("[{}] decrypting {} time-sdk={} ms", 
							uuid, adjustedLen, (nxl_endTime - nxl_startTime));
				}

				innerBlockCount += bufBlockCount;
			} catch (Exception ex) {
				LOGGER.error(ex.getMessage(), ex);
			}
		}

		return bosPlaintext.toByteArray();
	}
	
	@Override
	protected long getContentLength() {
		return nxlState.getContentLength();
	}

}
