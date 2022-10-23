/*
 * Created on November 19, 2019
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.decrypt.segment.server;

import java.nio.ByteBuffer;
import java.util.Stack;

import com.nextlabs.nxl.NxlFile;
import com.nextlabs.nxl.RightsManager;
import static com.nextlabs.common.shared.Constants.TokenGroupType.*;
import com.nextlabs.teamcenter.fms.configuration.NextLabsConfigInterface;
import com.nextlabs.teamcenter.fms.configuration.NextLabsConfigManager;
import com.nextlabs.teamcenter.fms.decrypt.segment.AbstractNextLabsDecryptingInputStream;
import com.nextlabs.teamcenter.fms.decrypt.segment.BytesRead;

import static com.nextlabs.teamcenter.fms.configuration.NextLabsConstants.*;
import com.teamcenter.fms.decrypt.segment.InterfaceBufferHandler;
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
public class NextLabsDecryptingInputStream extends AbstractNextLabsDecryptingInputStream {

	/** The NextLabs RMSDK state which is constructed from the header content. */
	private NxlFile nxlState;

	/** The NextLabs Rights Manager for handling the decryption. */
	private static RightsManager rightsManager;

	/** The NextLabs configuration for initialized Policy Controller Details. */
	private static final NextLabsConfigInterface NXLCONFIG;

	/** The version number */
	private static final String VERSION = NextLabsDecryptingInputStream.class
			.getName() + NXL_BUILD_VERSION;
	
	// TEST
	//private static Boolean flag = false;

	/**
	 * Static block: To initialize the static NextLabsConfigManager and
	 * NextLabsConfigInterface. Once initialized, the variables can be accessed
	 * by all the instances of NextLabsDecryptingInputStream.
	 */
	static {
		NextLabsConfigManager nxlConfigManager = NextLabsConfigManager
				.getService();

		NXLCONFIG = nxlConfigManager.getConfiguration();

		// Throw exception if the config has no data
		if (!NXLCONFIG.hasData()) {
			if (initExStack == null)
				initExStack = new Stack<Exception>();

			initExStack.add(new Exception("nxlconfig has no data"));
		}

		if (NXLCONFIG.hasData()) {
			// Initialize Rights Manager

			try {
				long nxl_startTime = System.currentTimeMillis();
				
				rightsManager = new RightsManager(NXLCONFIG.getRouterURL(), NXLCONFIG.getAppID(), NXLCONFIG.getAppKey());
				
				long io_endTime = System.currentTimeMillis();
				LOGGER.info("buildrm={} ms", (io_endTime - nxl_startTime));
			} catch (Exception ex) {
				if (initExStack == null)
					initExStack = new Stack<Exception>();
				
				initExStack.add(new Exception(
						"Failed to instantiate RightsManager"));

				rightsManager = null;
			}
		} else {
			rightsManager = null;
		}
	}

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
	public NextLabsDecryptingInputStream(
			final InterfaceFMSFileReaderStream fileAccessorStream) {
		super(fileAccessorStream);
		
		nxlState = null;
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
	public void init() throws Exception {
		long tempSourceFileLength = 0L;
		long tempPlaintextFileLength = 0L;

		// Get the source file size right away.
		// If not encrypted, there may not be enough source for the header.
		tempSourceFileLength = iSourceStream.fileSize();
		tempPlaintextFileLength = tempSourceFileLength;

		if ((fileHeader == null)
				&& (tempSourceFileLength >= HEADER_LENGTH)) {
			// We were not given a file header, but the source file is big
			// enough for a header. Try to read the header from the start
			// of the source stream.
			ByteBuffer potentialHeader = BB_POOLMANAGER.borrowObject();

			try {
				int bytesRead = iSourceStream.read(0L, potentialHeader,
						new InterfaceBufferHandler() {

							@Override
							public void onBuffer(final ByteBuffer bbData,
									final int iBufOffset, final int iBufSize)
									throws Exception {
								bbData.limit(iBufOffset + iBufSize);
								bbData.position(iBufOffset);
							}

						}, HEADER_LENGTH);

				if (bytesRead == HEADER_LENGTH) {
					fileHeader = new byte[HEADER_LENGTH];
					potentialHeader.get(fileHeader);

					if (!isHeaderEncrypted(fileHeader)) {
						LOGGER.debug("File is not encrypted");
						fileHeader = null;
					}
				}
			} catch (Exception ex) {
				LOGGER.error(ex.getMessage(), ex);
			} finally {
				BB_POOLMANAGER.returnObject(potentialHeader);
			}
		} // end of fileHeader == null

		// If fileHeader is populated, extract the plaintext length.
		if (rightsManager != null && fileHeader != null) {
			if (nxlState == null) {
				try {
					long nxl_startTime = System.currentTimeMillis();
					
					//if (!flag) {
						nxlState = rightsManager.buildNxlHeader(
								fileHeader, null, TOKENGROUP_SYSTEMBUCKET);
						//nxlState = rightsManager.buildNxlHeader(fileHeader);
					//	flag = true;
					//}
					
					long io_endTime = System.currentTimeMillis();
					LOGGER.info("buildNxlHeader={} ms", (io_endTime - nxl_startTime));
				} catch (Exception ex) {
					LOGGER.error(ex.getMessage(), ex);

					LOGGER.info("nxlState is set to null");
					nxlState = null;
				}
			}

			if (nxlState != null) {
				try {
					tempPlaintextFileLength = rightsManager
							.getOriginalContentLength(nxlState);
				} catch (Exception ex) {
					LOGGER.error(ex.getMessage(), ex);

					LOGGER.info("Failed to extract the plaintext length from nxlState");
				}
			}
		}

		sourceFileLength = tempSourceFileLength;
		plaintextFileLength = tempPlaintextFileLength;
		blockSize = (int) (sourceFileLength / NXL_BLOCK_SIZE);

		if (LOGGER.isDebugEnabled())
			LOGGER.debug("{} has been initialized sLen={} pLen={} encrypted={}", 
					getVersion(), sourceFileLength, plaintextFileLength, isEncrypted());
	}

	/**
	 * Request for the len that's needed for decryption based on NXL block size.
	 * This implementation is sending a new bb that may/may not fits requested
	 * len to downstream.
	 */
	@Override
	public int read(final long fileOffset, final ByteBuffer bb,
			final InterfaceBufferHandler buffHandler, final int len)
			throws Exception {
		
		long nxl_startTime = System.currentTimeMillis();

		if (LOGGER.isInfoEnabled())
			LOGGER.info("read() enters fileOffset={} bb.capacity={} bb.limit={} len={}", 
					fileOffset, bb.capacity(), bb.limit(), len);

		/* RETURNS PLAINTEXT */
		long sourceFileOffset = fileOffset;
		if (isEncrypted()) {
			sourceFileOffset += HEADER_LENGTH;
		}

		if (len <= 0) {
			if (LOGGER.isInfoEnabled()) {
				long nxl_endTime = System.currentTimeMillis();
				
				LOGGER.info("read() exits fileOffset={} bb.capacity={} bb.limit={} len={} time={} ms", 
						fileOffset, bb.capacity(), bb.limit(), len, (nxl_endTime - nxl_startTime));
			}

			return 0;
		}

		// len > 0
		// TODO: Do any necessary blocking here.

		// Read and buffer as much data as necessary to decrypt the
		// requested data.
		// If you can decrypt within the caller's buffer (b), that would
		// probably be faster
		// than double-buffering.
		/* READS CYPHERTEXT */
		// A single bulk read here would be preferred to reading individual
		// blocks.
		int bytesRead = 0;

		if (isEncrypted()) {
			if (initExStack != null && !initExStack.isEmpty())
				throw initExStack.get(0);

			// Calculate the blocks that need to read
			double startBlock = calcNextBlock(sourceFileOffset);
			double endBlock = calcNextBlock(sourceFileOffset + len);

			int blockCount = ((int) endBlock - (int) startBlock)
					+ (int) Math.ceil(Math.abs((endBlock - (int) endBlock)));

			if (LOGGER.isDebugEnabled())
				LOGGER.debug("fileOffset={} requestedLen={} sourceFileOffset={} startBlock={} endBlock={} blockCount={}", 
						fileOffset, len, sourceFileOffset, startBlock, endBlock, blockCount);

			// Allocate the bytebuffer based on NXL_BLOCK_SIZE
			final int bufferEncryptedSize = NXL_BLOCK_SIZE
					* BLOCK_MULTIPLIER;

			// TODO: Decrypt the data (in b), using fileHeader information.
			int lenEncrypted = blockCount * NXL_BLOCK_SIZE;
			if (LOGGER.isDebugEnabled())
				LOGGER.debug("requestedLen={} nxlRequestedLen={}", len, lenEncrypted);

			if (nxlState == null) {
				LOGGER.info("<WARNING> nxlState is null header={}", 
						new String(fileHeader));
				
				//if (!flag) {
					nxlState = rightsManager.buildNxlHeader(
							fileHeader, null, TOKENGROUP_SYSTEMBUCKET);
					//nxlState = rightsManager.buildNxlHeader(fileHeader);
				//	flag = true;
				//}

				if (nxlState == null) {
					LOGGER.error("<error> nxlState is null header={}", 
							new String(fileHeader));
				}
			}

			// Read the ciphertext by chunk in NXL block size
			final BytesRead bytesReadPlaintext = new BytesRead();
			InterfaceBufferHandler nxlBufferHandler = new NextLabsBufferHandlerServer(
					fileOffset, blockSize, blockCount, len,
					bufferEncryptedSize, sourceFileOffset, startBlock,
					bytesReadPlaintext, nxlState, rightsManager, bb,
					buffHandler, NXLCONFIG.getTenantName());

			ByteBuffer bufferEncrypted = BB_POOLMANAGER.borrowObject();
			if (LOGGER.isTraceEnabled())
				poolStatus(BB_POOLMANAGER, "BB_POOLMANAGER",
						"read() aft borrow");

			long io_startTime = System.currentTimeMillis();
			int bytesReadEncrypted = iSourceStream.read((int) startBlock
					* NXL_BLOCK_SIZE, bufferEncrypted,
					nxlBufferHandler, lenEncrypted); /* (RETURNS CIPHERTEXT) */
			long io_endTime = System.currentTimeMillis();
			LOGGER.info("time-io={} ms", (io_endTime - io_startTime));

			BB_POOLMANAGER.returnObject(bufferEncrypted);
			if (LOGGER.isTraceEnabled()) {
				poolStatus(BB_POOLMANAGER, "BB_POOLMANAGER",
						"read() aft return");
			}

			// set byteread to decrypted segment size
			bytesRead = bytesReadPlaintext.getLen();

			if (LOGGER.isDebugEnabled())
				LOGGER.debug("	>> bytesReadEncrypted={} bytesReadPlaintext={}", 
						bytesReadEncrypted, bytesRead);
		} else {
			if (LOGGER.isDebugEnabled())
				LOGGER.debug("Skip processing plaintext file ");

			bytesRead = iSourceStream.read(sourceFileOffset, bb, buffHandler,
					len); /* (RETURNS PLAINTEXT) */

			if (LOGGER.isDebugEnabled())
				LOGGER.debug("	>> bytesReadPlaintext={}", bytesRead);
		}

		if (LOGGER.isInfoEnabled()) {
			long nxl_endTime = System.currentTimeMillis();
			
			LOGGER.info("read() exits fileOffset={} bb.capacity={} bb.limit={} len={} bytesRead={} time-overall={} ms", 
					fileOffset, bb.capacity(), bb.limit(), len, bytesRead, (nxl_endTime - nxl_startTime));
		}

		return bytesRead;
	}

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
	public NextLabsDecryptingInputStream clone(
			final InterfaceFMSFileReaderStream fileAccessorStream)
			throws Exception {

		if (sourceFileLength < 0) {
			throw new Exception(
					"NextLabsDecryptingInputStream not initialized.");
		}

		NextLabsDecryptingInputStream newInstance = new NextLabsDecryptingInputStream(
				fileAccessorStream);

		// Share the file header, so we don't need to re-read it from the input
		// stream.
		newInstance.fileHeader = this.fileHeader;
		newInstance.nxlState = this.nxlState;
		// Initialize the cloned instance.
		newInstance.init();

		return newInstance;
	}

	/**
	 * Get version of the NextLabsDecryptingInputStream.
	 * 
	 * @return The version number.
	 */
	public static String getVersion() {
		return VERSION;
	}

}
