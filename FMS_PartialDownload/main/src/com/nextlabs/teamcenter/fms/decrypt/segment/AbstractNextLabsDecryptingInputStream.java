/*
 * Created on November 9, 2017
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.decrypt.segment;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Stack;

import org.apache.commons.pool2.impl.GenericObjectPool;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import static com.nextlabs.teamcenter.fms.configuration.NextLabsConstants.*;
import com.teamcenter.fms.decrypt.segment.InterfaceBufferHandler;
import com.teamcenter.fms.decrypt.segment.InterfaceDecryptingInputStream;
import com.teamcenter.fms.decrypt.segment.InterfaceFMSFileReaderStream;

/**
 * <p>
 * This stream reads a potentially NextLabs-encrypted file (from the
 * constructor-passed InterfaceFMSFileReaderStream) and decrypts its contents,
 * returning the appropriate file content, in plaintext, from its read(...)
 * methods.
 * <p>
 * NextLabs should complete the implementation of this class, and provide it to
 * Siemens PLM as a "third party library" (jar file).
 * <p>
 * Siemens PLM will provide a factory class that builds and manages these
 * streams as needed to process FMSClientCache requests.
 */
public abstract class AbstractNextLabsDecryptingInputStream implements
		InterfaceDecryptingInputStream {

	/** The source InterfaceFMSFileReaderStream (supplied during construction). */
	protected final InterfaceFMSFileReaderStream iSourceStream;

	/**
	 * The encryption header from the start of the encrypted file. Will be
	 * <code>null</code> if the source file is not encrypted, or if the stream
	 * has not yet been initialized.
	 */
	protected byte[] fileHeader;

	/** The number of bytes in the source file. */
	protected long sourceFileLength;

	/** The number of plaintext bytes in the source file (were it decrypted). */
	protected long plaintextFileLength;

	/** The number of blocks in the source file. */
	protected int blockSize;

	/** The error stack */
	protected static Stack<Exception> initExStack;

	/** The logger */
	protected static final Logger LOGGER = LogManager.getLogger("PARTIALDW_LOGGER");

	/** The pool manager for ByteBuffer */
	protected static final GenericObjectPool<ByteBuffer> BB_POOLMANAGER = new GenericObjectPool<ByteBuffer>(
			new ByteBufferFactory());
	

	/**
	 * Constructs an (uninitialized) NextLabsDecryptingInputStream. Once
	 * initialized, the; instance can access some or all of the (potentially
	 * encrypted) file using the InterfaceFMSFileReaderStream provided, and must
	 * supply plaintext content as a response to its read(...) methods.
	 * 
	 * @param fileAccessorStream
	 *            The (potentially encrypted) InterfaceFMSFileReaderStream
	 *            implementation.
	 */
	public AbstractNextLabsDecryptingInputStream(
			final InterfaceFMSFileReaderStream fileAccessorStream) {

		iSourceStream = fileAccessorStream;
		sourceFileLength = -1L;
		plaintextFileLength = -1L;

		fileHeader = null;
	}

	/**
	 * A separate initialization method, so that any Exceptions encountered
	 * during instance initialization can be cleaned up completely, and do not
	 * cause "partially constructed objects".
	 * 
	 * @throws Exception
	 *             If there is a problem initializing the stream.
	 */
	@Override
	public abstract void init() throws Exception;
	
	/** @Inherited */
	@Override
	public long plaintextFileSize() {
		//LOGGER.info("plaintextFileSize=" + plaintextFileLength);
		return plaintextFileLength;
	}

	/** @Inherited */
	@Override
	public boolean isEncrypted() {
		return (fileHeader != null);
	}

	/**
	 * Request for the len that's needed for decryption based on NXL block size.
	 * This implementation is sending a new bb that may/may not fits requested
	 * len to downstream.
	 */
	@Override
	public abstract int read(final long fileOffset, final ByteBuffer bb,
			final InterfaceBufferHandler buffHandler, final int len) throws Exception;
	
	/**
	 * Clones a NextLabsDecryptingInputStream, but with a new
	 * fileAccessorStream. The returned instance is useful for accessing the
	 * same file on a different thread, and avoiding any performance and
	 * interference issues associated with synchronization.
	 * 
	 * @param fileAccessorStream
	 *            The (potentially encrypted) InterfaceFMSFileReaderStream
	 *            implementation.
	 * @throws Exception
	 *             If there is a problem initializing the cloned stream.
	 */
	@Override
	public abstract AbstractNextLabsDecryptingInputStream clone(
			final InterfaceFMSFileReaderStream fileAccessorStream) throws Exception;
	
	/** @Inherited */
	@Override
	public void close() throws Exception {
		iSourceStream.close();
	}
	
	/**
	 * @return <code>true</code> if and only if the header is a valid encryption
	 *         header.
	 */
	protected boolean isHeaderEncrypted(byte[] header) {
		// TODO: Determine whether the file is actually encrypted.
		if (LOGGER.isTraceEnabled())
			LOGGER.trace("header={}", new String(header));

		try {
			// Copy the first 8 bytes from header
			byte[] headerNxlFormat = Arrays.copyOf(header,
					NXL_HEADER_FORMAT.length());

			// Convert the first 8 bytes to String
			String strHeaderNxlFormat = new String(headerNxlFormat);

			if (strHeaderNxlFormat != null
					&& strHeaderNxlFormat
							.equals(NXL_HEADER_FORMAT)) {
				return true;
			}
		} catch (NegativeArraySizeException ex) {
			LOGGER.error(ex.getMessage(), ex);
		} catch (NullPointerException ex) {
			LOGGER.error(ex.getMessage(), ex);
		}

		return false;
	}

	/**
	 * Calculate the block number that contains the requested offset.
	 * 
	 * @param offset
	 * @return The block number.
	 */
	protected double calcNextBlock(long offset) {
		double blockNum = offset / (double) NXL_BLOCK_SIZE;

		return blockNum;
	}

	/**
	 * Print the status of the pool manager.
	 * 
	 * @param poolManager
	 * @param poolManagerName
	 * @param prefix
	 */
	protected void poolStatus(GenericObjectPool<?> poolManager,
			String poolManagerName, String prefix) {

		LOGGER.info("	{} [{}] created count: {}", 
				prefix, poolManagerName, poolManager.getCreatedCount());
		LOGGER.info("	{} [{}] borrowed count: {}", 
				prefix, poolManagerName, poolManager.getBorrowedCount());
		LOGGER.info("	{} [{}] destroyed count: {}", 
				prefix, poolManagerName, poolManager.getDestroyedCount());
	}

}
