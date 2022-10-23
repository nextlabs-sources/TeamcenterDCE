/*
 * Created on November 9, 2017
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.decrypt.segment;

import java.nio.ByteBuffer;
import java.util.UUID;

import jakarta.xml.bind.DatatypeConverter;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

//import static com.nextlabs.teamcenter.fms.configuration.NextLabsConstants.*;
import static com.nextlabs.teamcenter.fms.configuration.NextLabsConstants.NXL_BLOCK_SIZE;
import static com.nextlabs.teamcenter.fms.configuration.NextLabsConstants.BLOCK_MULTIPLIER;
import static com.nextlabs.teamcenter.fms.configuration.NextLabsConstants.HEADER_LENGTH;

import com.teamcenter.fms.decrypt.segment.InterfaceBufferHandler;

public abstract class AbstractNextLabsBufferHandler implements InterfaceBufferHandler {

	/** The logger */
	protected static final Logger LOGGER = LogManager.getLogger("PARTIALDW_LOGGER");

	protected int innerBlockCount = 0;
	protected int innerLenRead = 0;

	private final long shiftedLen;
	private final int additionalBlock;

	private final long fileOffset;
	private final int blockSize;
	protected final int blockCount;
	private final int len;
	private final int bufferEncryptedSize;

	private final long sourceFileOffset;
	private final double startBlock;

	private final BytesRead bytesRead;
	private final ByteBuffer bb;

	private final InterfaceBufferHandler buffHandler;

	protected final String uuid;

	public AbstractNextLabsBufferHandler(long fileOffset, int blockSize,
			int blockCount, int len, int bufferEncryptedSize,
			long sourceFileOffset, double startBlock, BytesRead bytesRead,
			ByteBuffer bb, InterfaceBufferHandler buffHandler) {

		this.fileOffset = fileOffset;
		this.blockSize = blockSize;
		this.blockCount = blockCount;
		this.len = len;
		this.bufferEncryptedSize = bufferEncryptedSize;
		this.sourceFileOffset = sourceFileOffset;
		this.startBlock = startBlock;
		this.bytesRead = bytesRead;

		this.shiftedLen = this.sourceFileOffset % NXL_BLOCK_SIZE;
		// additionalBlock due to shifting
		this.additionalBlock = (this.shiftedLen > 0) ? 1 : 0;

		this.bb = bb;
		this.buffHandler = buffHandler;

		// generate and set uuid in debug mode only
		if (LOGGER.isDebugEnabled()) {
			this.uuid = UUID.randomUUID().toString();
			LOGGER.debug("{} links to {}", this.uuid, fileOffset);
		} else {
			this.uuid = "-";
		}
	}

	@Override
	public void onBuffer(final ByteBuffer bbData, final int iBufOffset,
			final int iBufSize) throws Exception {
		long buf_startTime = System.currentTimeMillis();

		if (LOGGER.isInfoEnabled())
			LOGGER.info("[{}] onBuffer({}, {}, {}) enters", 
					uuid, bbData.position(), iBufOffset, iBufSize);

		try {
			bbData.limit(iBufOffset + iBufSize);
		} catch (IllegalArgumentException ex) {
			LOGGER.error(
					"IllegalArgumentException while setting bbData's limit({} + {})", iBufOffset, iBufSize, ex);

			throw ex;
		}
		bbData.position(iBufOffset);

		byte[] data = new byte[iBufSize];
		bbData.get(data);
		bbData.clear();

		if (LOGGER.isTraceEnabled()) {
			LOGGER.trace("===			received raw data			===");
			LOGGER.trace(DatatypeConverter.printHexBinary(data));
			LOGGER.trace("===		end of received raw data		===");
		}

		if (LOGGER.isDebugEnabled())
			LOGGER.debug("[{}] encryptedDataLen={} requestedLen={}", uuid, data.length, len);

		long adjustedFileOffset = fileOffset;
		long adjustedLen;
		int bufBlockCount = iBufSize / NXL_BLOCK_SIZE;

		if (innerBlockCount == 0) {
			if (LOGGER.isDebugEnabled())
				LOGGER.debug("[{}] shiftedLen={}", uuid, shiftedLen);

			if (blockCount > BLOCK_MULTIPLIER) {
				adjustedLen = iBufSize - shiftedLen;
			} else {
				if (blockCount == 1)
					adjustedLen = len;
				else {
					adjustedLen = iBufSize - shiftedLen;

					if ((shiftedLen > 0)
							&& ((startBlock + bufBlockCount) >= blockSize)) {
						int paddedLen = calcPaddedLen(getContentLength());

						adjustedLen -= paddedLen;
					}
				}
			}

			adjustedLen = Math.min(adjustedLen, len);
		} else if ((((startBlock + innerBlockCount + bufBlockCount - 1) + additionalBlock) >= blockSize)
				|| (innerBlockCount + bufBlockCount >= blockCount)) {
			// last block in file or 2nd last block
			// last block in request
			adjustedFileOffset += innerLenRead;
			adjustedLen = (len - innerLenRead);
		} else {
			adjustedFileOffset += innerLenRead;
			adjustedLen = bufferEncryptedSize;
		}

		// request len more than plaintext len, minus padded
		if (adjustedFileOffset + adjustedLen > getContentLength()) {
			int paddedLen = calcPaddedLen(getContentLength());

			LOGGER.info("[{}] <WARNING> requested len > plaintext len adjustedLen({}) is changed to {}-{}={}", 
					uuid, adjustedLen, iBufSize, paddedLen, (iBufSize - paddedLen));
			
			adjustedLen = iBufSize - paddedLen;
		}

		innerLenRead += adjustedLen;
		bytesRead.setLen(innerLenRead);
		
		byte[] bufferPlaintext = processData(data, adjustedFileOffset, 
				adjustedLen, iBufSize, bufBlockCount);

		if (LOGGER.isTraceEnabled()) {
			LOGGER.trace("===			decrypted raw data			===");
			LOGGER.trace(DatatypeConverter.printHexBinary(bufferPlaintext));
			LOGGER.trace("===		end of decrpyted raw data		===");
		}

		if (LOGGER.isDebugEnabled()) {
			LOGGER.debug("[{}] decryptedDataLen={} requestedLen={}", uuid, bufferPlaintext.length, len);

			LOGGER.debug("[{}] bb.hasRemaining={} bb.position={} plainDataLen={}", 
					uuid, bb.hasRemaining(), bb.position(), bufferPlaintext.length);
		}

		if (!bb.hasRemaining() && bufferPlaintext.length > 0) {
			bb.flip();
			LOGGER.debug("[{}] aft flip bb.hasRemaining={}", uuid, bb.hasRemaining());
		}

		int byteIndex = 0;
		while (bb.hasRemaining()) {
			for (; byteIndex < bufferPlaintext.length && bb.hasRemaining(); byteIndex++) {
				byte plainData = bufferPlaintext[byteIndex];

				try {
					bb.put(plainData);
				} catch (Exception ex) {
					LOGGER.error("Exception while putting data to bb bb.limit={} plainDataLen={}", 
							bb.limit(), bufferPlaintext.length, ex);
					throw ex;
				}
			}

			int bbSize = bb.position();
			if (LOGGER.isDebugEnabled())
				LOGGER.debug("[{}] flushing bb ... [offset: {} size: {}]", 
						uuid, 0, bbSize);

			bb.flip();
			buffHandler.onBuffer(bb, 0, bbSize);
			bb.clear();

			if (byteIndex == bufferPlaintext.length) {
				break;
			}
		}

		if (LOGGER.isDebugEnabled())
			LOGGER.debug("[{}] request fulfilled [len: {}/{}]", 
					uuid, innerLenRead, len);
		
		if (LOGGER.isInfoEnabled()) {
			long buf_endTime = System.currentTimeMillis();
			
			LOGGER.info("[{}] onBuffer() exits time-buf={} ms", 
					uuid, (buf_endTime - buf_startTime));
		}
	}
	
	protected abstract long getContentLength();
	
	protected abstract byte[] processData(byte[] data, final long adjustedFileOffset, 
			final long adjustedLen, final int iBufSize, final int bufBlockCount);

	private int calcPaddedLen(long originalLen) {
		int lenPadded = (int) ((originalLen + HEADER_LENGTH) % NXL_BLOCK_SIZE);
		
		if (lenPadded <= 0)
			return 0;
		else
			return NXL_BLOCK_SIZE - lenPadded;
//		return (int) (NextLabsConstants.NXL_BLOCK_SIZE - (originalLen + NextLabsConstants.HEADER_LENGTH)
//				% NextLabsConstants.NXL_BLOCK_SIZE);
	}

}
