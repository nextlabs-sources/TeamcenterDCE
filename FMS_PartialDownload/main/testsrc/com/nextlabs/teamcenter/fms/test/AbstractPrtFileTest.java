package com.nextlabs.teamcenter.fms.test;

import static org.junit.Assert.assertEquals;
import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.nio.ByteBuffer;

//import org.apache.log4j.Logger;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.junit.Test;

import com.nextlabs.teamcenter.fms.decrypt.segment.AbstractNextLabsDecryptingInputStream;
import com.nextlabs.teamcenter.fms.test.TestFMSFileReaderStream;
import com.teamcenter.fms.decrypt.segment.InterfaceBufferHandler;
import static com.nextlabs.teamcenter.fms.test.TestConstants.*;

/**
 * 
 * @author clow
 * Test cases for plaintext Prt file and encrypted Prt file
 * 0. Assert the plaintext file size
 * 1. Assert random access case 1, fileoffset: 4063, off: 0, len: 32
 * 2. Assert random access case 2, fileoffset: 4094, off: 0, len: 64
 * 3. Assert random access case 3, fileoffset: 4063, off: 0, len: 32; fileoffset: 0, off: 0, len: 32 
 * 4. Assert random access case 4, fileoffset: 11301, off: 0, len: 1315  
 * 5. Assert random access case 5, fileoffset: 11301, off: 0, len: 5096
 * 6. Assert random access case 6, fileoffset: 8051, off: 0, len: 10240
 * 7. Assert random access case 7 [plaintext], fileoffset: 1024, off: 0, len: 125000
 * 8. Assert random access case 8, fileoffset: 8192, off: 0, len: 1024000
 * 9. Assert random access case 9, fileoffset: 14502224, off: 0, len: 1024000
 *
 */

public abstract class AbstractPrtFileTest {
	
	private File plaintextFile = new File(PRT_PLAINTEXTFILE_PATH);
	private File encryptedFile = new File(PRT_ENCRYPTEDFILE_PATH);
	private static boolean SHOWDATA = false;
	
	private static final Logger LOGGER = LogManager.getLogger("PARTIALDW_TEST_LOGGER");
	
	static {
		if (LOGGER.isDebugEnabled())
			SHOWDATA = true;
	}
	
	protected abstract AbstractNextLabsDecryptingInputStream 
		getNextLabsDecryptingInputStream(TestFMSFileReaderStream fisEncrypted);
	
	/**
	 * assert plaintext file size
	 */
	@Test
	public void assertPlaintextFileSize() throws Exception {
		TestFMSFileReaderStream fisEncrypted = new TestFMSFileReaderStream(
				new FileInputStream(encryptedFile));
		
		AbstractNextLabsDecryptingInputStream nxlDecryptingInputStream = 
				getNextLabsDecryptingInputStream(fisEncrypted);
		nxlDecryptingInputStream.init();
		
		long plaintextFileSize = nxlDecryptingInputStream.plaintextFileSize();
		nxlDecryptingInputStream.close();

		// Test result
		LOGGER.info("0. Start [{}] => Assert plaintext filesize", this.getClass().getSimpleName());
		LOGGER.info("	plaintext filesize: {}", plaintextFile.length());
		LOGGER.info("	encrypted plaintext filesize: {}", plaintextFileSize);
		LOGGER.info("0. End => Assert plaintext filesize");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(plaintextFile.length(), plaintextFileSize);
	}
	
	/**
	 * assert random access case 1
	 * fileoffset: 4063, off: 0, len: 32
	 */
	@Test
	public void assertRandomAccess1() throws Exception {
		// test case configuration
		int len = 32;
		int bbSize = 32;
		int fileOffset = 4063;
		int offset = 0;
		
		long startPlaintext = System.nanoTime();
		FileInputStream fis = new FileInputStream(plaintextFile);
		BufferedInputStream bis = new BufferedInputStream(fis);
		byte[] bufferPlaintext = new byte[len + offset];
		
		bis.mark(fileOffset + len);
		
		// skip/seek forward 
		long totalSkipped = bis.skip(fileOffset);
		while (totalSkipped < fileOffset) {
			totalSkipped += bis.skip(fileOffset - totalSkipped);
		}
		
		int plaintextByteRead = bis.read(bufferPlaintext, offset, len);
		bis.close();
		fis.close();
		long endPlaintext = System.nanoTime();
		
		StringBuilder plaintextData = new StringBuilder();
		for (byte b : bufferPlaintext) {
			plaintextData.append(b + " ");
		}
		
		
		// Access protected file
		long startEncrypted = System.nanoTime();
		TestFMSFileReaderStream fisProtected = new TestFMSFileReaderStream(
				new FileInputStream(encryptedFile));
		AbstractNextLabsDecryptingInputStream nxlDecryptingInputStream = 
				getNextLabsDecryptingInputStream(fisProtected);
		nxlDecryptingInputStream.init();
				
		ByteBuffer bufferEncrypted = ByteBuffer.allocateDirect(bbSize);
		final StringBuilder strBldrDecrypted = new StringBuilder();
		
		int encryptedByteRead = nxlDecryptingInputStream.read(
				fileOffset, bufferEncrypted, new InterfaceBufferHandler() {
					public void onBuffer(final ByteBuffer bbData, final int iBufOffset, final int iBufSize) throws Exception {
						LOGGER.trace("onBuffer() enters");
						bbData.limit(iBufOffset + iBufSize);
						bbData.position(iBufOffset);
						
						byte[] data = new byte[iBufSize];
						bbData.get(data);
						
						for (byte b : data) {
							strBldrDecrypted.append(b + " ");
						}						
						
						bbData.clear();
						LOGGER.trace("onBuffer() exits");
					}
				}, len);
		nxlDecryptingInputStream.close();
		long endEncrypted = System.nanoTime();
				
		// Test result
		LOGGER.info("1. Start [{}] => Assert random access 1 (fileoffset:{}, off:{},len:{})", 
				this.getClass().getSimpleName(), fileOffset, offset, len);
		LOGGER.info("	plaintext byteread: {}", plaintextByteRead);
		LOGGER.info("	encrypted byteread: {}", encryptedByteRead);
		LOGGER.info("	plaintext read(us): {}", (endPlaintext - startPlaintext) / 1000);
		LOGGER.info("	encrypted read(us): {}", (endEncrypted - startEncrypted) / 1000);
		if (SHOWDATA) {
			LOGGER.info("=============		PLAINTEXT DATA		============={}{}", 
					System.lineSeparator(), plaintextData.toString());
			LOGGER.info("=============		DECRYPTED DATA		============={}{}", 
					System.lineSeparator(), strBldrDecrypted.toString());
		}
		LOGGER.info("1. End => Assert random access 1");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(plaintextData.toString(), strBldrDecrypted.toString());
	}
	
	/**
	 * assert random access case 2
	 * fileoffset: 4094, off: 0, len: 64
	 */
	@Test
	public void assertRandomAccess2() throws Exception {
		// test case configuration
		int len = 64;
		int bbSize = 32;
		int fileOffset = 4094;
		int offset = 0;
		
		long startPlaintext = System.nanoTime();
		FileInputStream fis = new FileInputStream(plaintextFile);
		BufferedInputStream bis = new BufferedInputStream(fis);
		byte[] bufferPlaintext = new byte[len + offset];
		
		bis.mark(fileOffset + len);
		
		// skip/seek forward 
		long totalSkipped = bis.skip(fileOffset);
		while (totalSkipped < fileOffset) {
			totalSkipped += bis.skip(fileOffset - totalSkipped);
		}
		
		int plaintextByteRead = bis.read(bufferPlaintext, offset, len);
		bis.close();
		fis.close();
		long endPlaintext = System.nanoTime();
		
		StringBuilder plaintextData = new StringBuilder();
		for (byte b : bufferPlaintext) {
			plaintextData.append(b + " ");
		}

		
		// Access protected file
		long startEncrypted = System.nanoTime();
		TestFMSFileReaderStream fisProtected = new TestFMSFileReaderStream(
				new FileInputStream(encryptedFile));
		AbstractNextLabsDecryptingInputStream nxlDecryptingInputStream = 
				getNextLabsDecryptingInputStream(fisProtected);
		nxlDecryptingInputStream.init();
				
		ByteBuffer bufferEncrypted = ByteBuffer.allocateDirect(bbSize);
		final StringBuilder strBldrDecrypted = new StringBuilder();
		
		int encryptedByteRead = nxlDecryptingInputStream.read(
				fileOffset, bufferEncrypted, new InterfaceBufferHandler() {
					public void onBuffer(final ByteBuffer bbData, final int iBufOffset, final int iBufSize) throws Exception {
						LOGGER.trace("onBuffer() enters");
						bbData.limit(iBufOffset + iBufSize);
						bbData.position(iBufOffset);
						
						byte[] data = new byte[iBufSize];
						bbData.get(data);
						
						for (byte b : data) {
							strBldrDecrypted.append(b + " ");
						}						
						
						bbData.clear();
						LOGGER.trace("onBuffer() exits");
					}
				}, len);
		nxlDecryptingInputStream.close();
		long endEncrypted = System.nanoTime();
		
		// Test result
		LOGGER.info("2. Start [{}] => Assert random access 2 (fileoffset:{}, off:{},len:{})", 
				this.getClass().getSimpleName(), fileOffset, offset, len);
		LOGGER.info("	plaintext byteread: {}", plaintextByteRead);
		LOGGER.info("	encrypted byteread: {}", encryptedByteRead);
		LOGGER.info("	plaintext read(us): {}", (endPlaintext - startPlaintext) / 1000);
		LOGGER.info("	encrypted read(us): {}", (endEncrypted - startEncrypted) / 1000);
		if (SHOWDATA) {
			LOGGER.info("=============		PLAINTEXT DATA		============={}{}", 
					System.lineSeparator(), plaintextData.toString());
			LOGGER.info("=============		DECRYPTED DATA		============={}{}", 
					System.lineSeparator(), strBldrDecrypted.toString());
		}
		LOGGER.info("2. End => Assert random access 2");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(plaintextData.toString(), strBldrDecrypted.toString());
	}
	
	/**
	 * assert random access case 3
	 * fileoffset: 4063, off: 0, len: 32
	 * fileoffset: 0, off: 0, len: 32
	 */
	@Test
	public void assertRandomAccess3() throws Exception {
		// test case configuration
		int len = 32;
		int bbSize = 16;
		int fileOffset1 = 4063;
		int fileOffset2 = 0;
		int offset = 0;
		
		long startPlaintext = System.nanoTime();
		FileInputStream fis = new FileInputStream(plaintextFile);
		BufferedInputStream bis = new BufferedInputStream(fis);
		byte[] bufferPlaintext1 = new byte[len + offset];
		byte[] bufferPlaintext2 = new byte[len + offset];
		
		bis.mark(fileOffset1 + len);
		
		// skip/seek forward 
		long totalSkipped1 = bis.skip(fileOffset1);
		while (totalSkipped1 < fileOffset1) {
			totalSkipped1 += bis.skip(fileOffset1 - totalSkipped1);
		}
		
		int plaintextByteRead1 = bis.read(bufferPlaintext1, offset, len);
		bis.reset();
		
		// skip/seek forward
		bis.mark(fileOffset2 + len);
		long totalSkipped2 = bis.skip(fileOffset2);
		while (totalSkipped2 < fileOffset2) {
			totalSkipped2 += bis.skip(fileOffset2 - totalSkipped2);
		}
		
		int plaintextByteRead2 = bis.read(bufferPlaintext2, offset, len);
		bis.close();
		fis.close();
		long endPlaintext = System.nanoTime();
		
		StringBuilder plaintextData1 = new StringBuilder();
		for (byte b : bufferPlaintext1) {
			plaintextData1.append(b + " ");
		}
		
		StringBuilder plaintextData2 = new StringBuilder();
		for (byte b : bufferPlaintext2) {
			plaintextData2.append(b + " ");
		}
		

		// Access protected file
		long startEncrypted = System.nanoTime();
		TestFMSFileReaderStream fisProtected = new TestFMSFileReaderStream(
				new FileInputStream(encryptedFile));
		AbstractNextLabsDecryptingInputStream nxlDecryptingInputStream = 
				getNextLabsDecryptingInputStream(fisProtected);
		nxlDecryptingInputStream.init();
				
		ByteBuffer bufferEncrypted1 = ByteBuffer.allocateDirect(bbSize);
		final StringBuilder strBldrDecrypted1 = new StringBuilder();
				
		int encryptedByteRead1 = nxlDecryptingInputStream.read(
				fileOffset1, bufferEncrypted1, new InterfaceBufferHandler() {
					public void onBuffer(final ByteBuffer bbData, final int iBufOffset, final int iBufSize) throws Exception {
						LOGGER.trace("onBuffer() enters");
						bbData.limit(iBufOffset + iBufSize);
						bbData.position(iBufOffset);
						
						byte[] data = new byte[iBufSize];
						bbData.get(data);
						
						for (byte b : data) {
							strBldrDecrypted1.append(b + " ");
						}						
						
						bbData.clear();
						LOGGER.trace("onBuffer() exits");
					}
				}, len);
		
		ByteBuffer bufferEncrypted2 = ByteBuffer.allocateDirect(bbSize);
		final StringBuilder strBldrDecrypted2 = new StringBuilder();
		
		int encryptedByteRead2 = nxlDecryptingInputStream.read(
				fileOffset2, bufferEncrypted2, new InterfaceBufferHandler() {
					public void onBuffer(final ByteBuffer bbData, final int iBufOffset, final int iBufSize) throws Exception {
						LOGGER.trace("onBuffer() enters");
						bbData.limit(iBufOffset + iBufSize);
						bbData.position(iBufOffset);
						
						byte[] data = new byte[iBufSize];
						bbData.get(data);
						
						for (byte b : data) {
							strBldrDecrypted2.append(b + " ");
						}						
						
						bbData.clear();
						LOGGER.trace("onBuffer() exits");
					}
				}, len);
		nxlDecryptingInputStream.close();
		long endEncrypted = System.nanoTime();
		
		// Test result
		LOGGER.info("3. Start [{}] => Assert random access 3 (fileoffset:{}, off:{},len:{})", 
				this.getClass().getSimpleName(), fileOffset1, offset, len);
		LOGGER.info("	plaintext byteread: {}", plaintextByteRead1);
		LOGGER.info("	encrypted byteread: {}", encryptedByteRead1);
		if (SHOWDATA) {
			LOGGER.info("=============		PLAINTEXT DATA		============={}{}", 
					System.lineSeparator(), plaintextData1.toString());
			LOGGER.info("=============		DECRYPTED DATA		============={}{}", 
					System.lineSeparator(), strBldrDecrypted1.toString());
		}
		LOGGER.info("3. End => Assert random access 3");
		LOGGER.info("");
		LOGGER.info("3. Start [{}] => Assert random access 3 (fileoffset:{}, off:{},len:{})", 
				this.getClass().getSimpleName(), fileOffset2, offset, len);
		LOGGER.info("	plaintext byteread: {}", plaintextByteRead2);
		LOGGER.info("	encrypted byteread: {}", encryptedByteRead2);
		LOGGER.info("	plaintext read(us): {}", (endPlaintext - startPlaintext) / 1000);
		LOGGER.info("	encrypted read(us): {}", (endEncrypted - startEncrypted) / 1000);
		if (SHOWDATA) {
			LOGGER.info("=============		PLAINTEXT DATA		============={}{}", 
					System.lineSeparator(), plaintextData2.toString());
			LOGGER.info("=============		DECRYPTED DATA		============={}{}", 
					System.lineSeparator(), strBldrDecrypted2.toString());
		}
		LOGGER.info("3. End => Assert random access 3");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(plaintextByteRead1, encryptedByteRead1);
		assertEquals(plaintextData1.toString(), strBldrDecrypted1.toString());
		assertEquals(plaintextByteRead2, encryptedByteRead2);
		assertEquals(plaintextData2.toString(), strBldrDecrypted2.toString());
	}
	
	/**
	 * assert random access case 4
	 * fileoffset: 11301, off: 0, len: 1315
	 */
	@Test
	public void assertRandomAccess4() throws Exception {
		// test case configuration
		int len = 1315;
		int bbSize = 64;
		int fileOffset = 11301;
		int offset = 0;
		
		long startPlaintext = System.nanoTime();
		FileInputStream fis = new FileInputStream(plaintextFile);
		BufferedInputStream bis = new BufferedInputStream(fis);
		byte[] bufferPlaintext = new byte[len + offset];
		
		bis.mark(fileOffset + len);
		
		// skip/seek forward 
		long totalSkipped = bis.skip(fileOffset);
		while (totalSkipped < fileOffset) {
			totalSkipped += bis.skip(fileOffset - totalSkipped);
		}
		
		int plaintextByteRead = bis.read(bufferPlaintext, offset, len);
		bis.close();
		fis.close();
		long endPlaintext = System.nanoTime();
		
		StringBuilder plaintextData = new StringBuilder();
		for (byte b : bufferPlaintext) {
			plaintextData.append(b + " ");
		}
		
		
		// Access protected file
		long startEncrypted = System.nanoTime();
		TestFMSFileReaderStream fisProtected = new TestFMSFileReaderStream(
				new FileInputStream(encryptedFile));
		AbstractNextLabsDecryptingInputStream nxlDecryptingInputStream = 
				getNextLabsDecryptingInputStream(fisProtected);
		nxlDecryptingInputStream.init();
				
		ByteBuffer bufferEncrypted = ByteBuffer.allocateDirect(bbSize);
		final StringBuilder strBldrDecrypted = new StringBuilder();
		
		int encryptedByteRead = nxlDecryptingInputStream.read(
				fileOffset, bufferEncrypted, new InterfaceBufferHandler() {
					public void onBuffer(final ByteBuffer bbData, final int iBufOffset, final int iBufSize) throws Exception {
						LOGGER.trace("onBuffer() enters");
						bbData.limit(iBufOffset + iBufSize);
						bbData.position(iBufOffset);
						
						byte[] data = new byte[iBufSize];
						bbData.get(data);
						
						for (byte b : data) {
							strBldrDecrypted.append(b + " ");
						}						
						
						bbData.clear();
						LOGGER.trace("onBuffer() exits");
					}
				}, len);
		nxlDecryptingInputStream.close();
		long endEncrypted = System.nanoTime();
				
		// Test result
		LOGGER.info("4. Start [{}] => Assert random access 4 (fileoffset:{}, off:{},len:{})", 
				this.getClass().getSimpleName(), fileOffset, offset, len);
		LOGGER.info("	plaintext byteread: {}", plaintextByteRead);
		LOGGER.info("	encrypted byteread: {}", encryptedByteRead);
		LOGGER.info("	plaintext read(us): {}", (endPlaintext - startPlaintext) / 1000);
		LOGGER.info("	encrypted read(us): {}", (endEncrypted - startEncrypted) / 1000);
		if (SHOWDATA) {
			LOGGER.info("=============		PLAINTEXT DATA		============={}{}", 
					System.lineSeparator(), plaintextData.toString());
			LOGGER.info("=============		DECRYPTED DATA		============={}{}", 
					System.lineSeparator(), strBldrDecrypted.toString());
		}
		LOGGER.info("4. End => Assert random access 4");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(plaintextData.toString(), strBldrDecrypted.toString());
	}
	
	/**
	 * assert random access case 5
	 * fileoffset: 11301, off: 0, len: 5096
	 */
	@Test
	public void assertRandomAccess5() throws Exception {
		// test case configuration
		int len = 5096;
		int bbSize = 64;
		int fileOffset = 11301;
		int offset = 0;
		
		long startPlaintext = System.nanoTime();
		FileInputStream fis = new FileInputStream(plaintextFile);
		BufferedInputStream bis = new BufferedInputStream(fis);
		byte[] bufferPlaintext = new byte[len + offset];
		
		bis.mark(fileOffset + len);
		
		// skip/seek forward 
		long totalSkipped = bis.skip(fileOffset);
		while (totalSkipped < fileOffset) {
			totalSkipped += bis.skip(fileOffset - totalSkipped);
		}
		
		int plaintextByteRead = bis.read(bufferPlaintext, offset, len);
		bis.close();
		fis.close();
		long endPlaintext = System.nanoTime();
		
		StringBuilder plaintextData = new StringBuilder();
		for (byte b : bufferPlaintext) {
			plaintextData.append(b + " ");
		}
		
		
		// Access protected file
		long startEncrypted = System.nanoTime();
		TestFMSFileReaderStream fisProtected = new TestFMSFileReaderStream(
				new FileInputStream(encryptedFile));
		AbstractNextLabsDecryptingInputStream nxlDecryptingInputStream = 
				getNextLabsDecryptingInputStream(fisProtected);
		nxlDecryptingInputStream.init();
				
		ByteBuffer bufferEncrypted = ByteBuffer.allocateDirect(bbSize);
		final StringBuilder strBldrDecrypted = new StringBuilder();
		
		int encryptedByteRead = nxlDecryptingInputStream.read(
				fileOffset, bufferEncrypted, new InterfaceBufferHandler() {
					public void onBuffer(final ByteBuffer bbData, final int iBufOffset, final int iBufSize) throws Exception {
						LOGGER.trace("onBuffer() enters");
						bbData.limit(iBufOffset + iBufSize);
						bbData.position(iBufOffset);
						
						byte[] data = new byte[iBufSize];
						bbData.get(data);
						
						for (byte b : data) {
							strBldrDecrypted.append(b + " ");
						}						
						
						bbData.clear();
						LOGGER.trace("onBuffer() exits");
					}
				}, len);
		nxlDecryptingInputStream.close();
		long endEncrypted = System.nanoTime();
				
		// Test result
		LOGGER.info("5. Start [{}] => Assert random access 5 (fileoffset:{}, off:{},len:{})", 
				this.getClass().getSimpleName(), fileOffset, offset, len);
		LOGGER.info("	plaintext byteread: {}", plaintextByteRead);
		LOGGER.info("	encrypted byteread: {}", encryptedByteRead);
		LOGGER.info("	plaintext read(us): {}", (endPlaintext - startPlaintext) / 1000);
		LOGGER.info("	encrypted read(us): {}", (endEncrypted - startEncrypted) / 1000);
		if (SHOWDATA) {
			LOGGER.info("=============		PLAINTEXT DATA		============={}{}", 
					System.lineSeparator(), plaintextData.toString());
			LOGGER.info("=============		DECRYPTED DATA		============={}{}", 
					System.lineSeparator(), strBldrDecrypted.toString());
		}
		LOGGER.info("5. End => Assert random access 5");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(plaintextData.toString(), strBldrDecrypted.toString());
	}
	
	/**
	 * assert random access case 6
	 * fileoffset: 8051, off: 0, len: 10240
	 */
	@Test
	public void assertRandomAccess6() throws Exception {
		// test case configuration
		int len = 10240;
		int bbSize = 64;
		int fileOffset = 8051;
		int offset = 0;
		
		long startPlaintext = System.nanoTime();
		FileInputStream fis = new FileInputStream(plaintextFile);
		BufferedInputStream bis = new BufferedInputStream(fis);
		byte[] bufferPlaintext = new byte[len + offset];
		
		bis.mark(fileOffset + len);
		
		// skip/seek forward 
		long totalSkipped = bis.skip(fileOffset);
		while (totalSkipped < fileOffset) {
			totalSkipped += bis.skip(fileOffset - totalSkipped);
		}
		
		int plaintextByteRead = bis.read(bufferPlaintext, offset, len);
		bis.close();
		fis.close();
		long endPlaintext = System.nanoTime();
		
		StringBuilder plaintextData = new StringBuilder();
		for (byte b : bufferPlaintext) {
			plaintextData.append(b + " ");
		}
		
		
		// Access protected file
		long startEncrypted = System.nanoTime();
		TestFMSFileReaderStream fisProtected = new TestFMSFileReaderStream(
				new FileInputStream(encryptedFile));
		AbstractNextLabsDecryptingInputStream nxlDecryptingInputStream = 
				getNextLabsDecryptingInputStream(fisProtected);
		nxlDecryptingInputStream.init();
				
		ByteBuffer bufferEncrypted = ByteBuffer.allocateDirect(bbSize);
		final StringBuilder strBldrDecrypted = new StringBuilder();
		
		int encryptedByteRead = nxlDecryptingInputStream.read(
				fileOffset, bufferEncrypted, new InterfaceBufferHandler() {
					public void onBuffer(final ByteBuffer bbData, final int iBufOffset, final int iBufSize) throws Exception {
						LOGGER.trace("onBuffer() enters");
						bbData.limit(iBufOffset + iBufSize);
						bbData.position(iBufOffset);
						
						byte[] data = new byte[iBufSize];
						bbData.get(data);
						
						for (byte b : data) {
							strBldrDecrypted.append(b + " ");
						}						
						
						bbData.clear();
						LOGGER.trace("onBuffer() exits");
					}
				}, len);
		nxlDecryptingInputStream.close();
		long endEncrypted = System.nanoTime();
				
		// Test result
		LOGGER.info("6. Start [{}] => Assert random access 6 (fileoffset:{}, off:{},len:{})", 
				this.getClass().getSimpleName(), fileOffset, offset, len);
		LOGGER.info("	plaintext byteread: {}", plaintextByteRead);
		LOGGER.info("	encrypted byteread: {}", encryptedByteRead);
		LOGGER.info("	plaintext read(us): {}", (endPlaintext - startPlaintext) / 1000);
		LOGGER.info("	encrypted read(us): {}", (endEncrypted - startEncrypted) / 1000);
		if (SHOWDATA) {
			LOGGER.info("=============		PLAINTEXT DATA		============={}{}", 
					System.lineSeparator(), plaintextData.toString());
			LOGGER.info("=============		DECRYPTED DATA		============={}{}", 
					System.lineSeparator(), strBldrDecrypted.toString());
		}
		LOGGER.info("6. End => Assert random access 6");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(plaintextData.toString(), strBldrDecrypted.toString());
	}

	/**
	 * assert random access case 7
	 * fileoffset: 1024, off: 0, len: 125000
	 */
	@Test
	public void assertRandomAccess7() throws Exception {
		// test case configuration
		int len = 125000;
		int bbSize = 64;
		int fileOffset = 1024;
		int offset = 0;
		
		long startPlaintext = System.nanoTime();
		FileInputStream fis = new FileInputStream(plaintextFile);
		BufferedInputStream bis = new BufferedInputStream(fis);
		byte[] bufferPlaintext = new byte[len + offset];
		
		bis.mark(fileOffset + len);
		
		// skip/seek forward 
		long totalSkipped = bis.skip(fileOffset);
		while (totalSkipped < fileOffset) {
			totalSkipped += bis.skip(fileOffset - totalSkipped);
		}
		
		int plaintextByteRead = bis.read(bufferPlaintext, offset, len);
		bis.close();
		fis.close();
		long endPlaintext = System.nanoTime();
		
		StringBuilder plaintextData = new StringBuilder();
		for (byte b : bufferPlaintext) {
			plaintextData.append(b + " ");
		}
		
		
		// Access protected file
		long startEncrypted = System.nanoTime();
		TestFMSFileReaderStream fisProtected = new TestFMSFileReaderStream(
				new FileInputStream(plaintextFile));
		AbstractNextLabsDecryptingInputStream nxlDecryptingInputStream = 
				getNextLabsDecryptingInputStream(fisProtected);
		nxlDecryptingInputStream.init();
				
		ByteBuffer bufferEncrypted = ByteBuffer.allocateDirect(bbSize);
		final StringBuilder strBldrDecrypted = new StringBuilder();
		
		int encryptedByteRead = nxlDecryptingInputStream.read(
				fileOffset, bufferEncrypted, new InterfaceBufferHandler() {
					public void onBuffer(final ByteBuffer bbData, final int iBufOffset, final int iBufSize) throws Exception {
						LOGGER.trace("onBuffer() enters");
						bbData.limit(iBufOffset + iBufSize);
						bbData.position(iBufOffset);
						
						byte[] data = new byte[iBufSize];
						bbData.get(data);
						
						for (byte b : data) {
							strBldrDecrypted.append(b + " ");
						}						
						
						bbData.clear();
						LOGGER.trace("onBuffer() exits");
					}
				}, len);
		nxlDecryptingInputStream.close();
		long endEncrypted = System.nanoTime();
				
		// Test result
		LOGGER.info("7. Start [{}] => Assert random access 7 (fileoffset:{}, off:{},len:{})", 
				this.getClass().getSimpleName(), fileOffset, offset, len);
		LOGGER.info("	plaintext byteread: {}", plaintextByteRead);
		LOGGER.info("	encrypted byteread: {}", encryptedByteRead);
		LOGGER.info("	plaintext read(us): {}", (endPlaintext - startPlaintext) / 1000);
		LOGGER.info("	encrypted read(us): {}", (endEncrypted - startEncrypted) / 1000);
		if (SHOWDATA) {
			LOGGER.info("=============		PLAINTEXT DATA		============={}{}", 
					System.lineSeparator(), plaintextData.toString());
			LOGGER.info("=============		DECRYPTED DATA		============={}{}", 
					System.lineSeparator(), strBldrDecrypted.toString());
		}
		LOGGER.info("7. End => Assert random access 7");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(plaintextData.toString(), strBldrDecrypted.toString());
	}
	
	/**
	 * assert random access case 8
	 * fileoffset: 8192, off: 0, len: 1024000
	 */
	@Test
	public void assertRandomAccess8() throws Exception {
		// test case configuration
		int len = 1024000;
		int bbSize = 64;
		int fileOffset = 8192;
		int offset = 0;
		
		long startPlaintext = System.nanoTime();
		FileInputStream fis = new FileInputStream(plaintextFile);
		BufferedInputStream bis = new BufferedInputStream(fis);
		byte[] bufferPlaintext = new byte[len + offset];
		
		bis.mark(fileOffset + len);
		
		// skip/seek forward 
		long totalSkipped = bis.skip(fileOffset);
		while (totalSkipped < fileOffset) {
			totalSkipped += bis.skip(fileOffset - totalSkipped);
		}
		
		int plaintextByteRead = bis.read(bufferPlaintext, offset, len);
		bis.close();
		fis.close();
		long endPlaintext = System.nanoTime();
		
		StringBuilder plaintextData = new StringBuilder();
		for (byte b : bufferPlaintext) {
			plaintextData.append(b + " ");
		}
		
		
		// Access protected file
		long startEncrypted = System.nanoTime();
		TestFMSFileReaderStream fisProtected = new TestFMSFileReaderStream(
				new FileInputStream(encryptedFile));
		AbstractNextLabsDecryptingInputStream nxlDecryptingInputStream = 
				getNextLabsDecryptingInputStream(fisProtected);
		nxlDecryptingInputStream.init();
				
		ByteBuffer bufferEncrypted = ByteBuffer.allocateDirect(bbSize);
		final StringBuilder strBldrDecrypted = new StringBuilder();
		
		int encryptedByteRead = nxlDecryptingInputStream.read(
				fileOffset, bufferEncrypted, new InterfaceBufferHandler() {
					public void onBuffer(final ByteBuffer bbData, final int iBufOffset, final int iBufSize) throws Exception {
						LOGGER.trace("onBuffer() enters");
						bbData.limit(iBufOffset + iBufSize);
						bbData.position(iBufOffset);
						
						byte[] data = new byte[iBufSize];
						bbData.get(data);
						
						for (byte b : data) {
							strBldrDecrypted.append(b + " ");
						}						
						
						bbData.clear();
						LOGGER.trace("onBuffer() exits");
					}
				}, len);
		nxlDecryptingInputStream.close();
		long endEncrypted = System.nanoTime();
				
		// Test result
		LOGGER.info("8. Start [{}] => Assert random access 8 (fileoffset:{}, off:{},len:{})", 
				this.getClass().getSimpleName(), fileOffset, offset, len);
		LOGGER.info("	plaintext byteread: {}", plaintextByteRead);
		LOGGER.info("	encrypted byteread: {}", encryptedByteRead);
		LOGGER.info("	plaintext read(us): {}", (endPlaintext - startPlaintext) / 1000);
		LOGGER.info("	encrypted read(us): {}", (endEncrypted - startEncrypted) / 1000);
		if (SHOWDATA) {
			LOGGER.info("=============		PLAINTEXT DATA		============={}{}", 
					System.lineSeparator(), plaintextData.toString());
			LOGGER.info("=============		DECRYPTED DATA		============={}{}", 
					System.lineSeparator(), strBldrDecrypted.toString());
		}
		LOGGER.info("8. End => Assert random access 8");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(plaintextData.toString(), strBldrDecrypted.toString());
	}
	
	/**
	 * assert random access case 9
	 * fileoffset: 14502224, off: 0, len: 1024000
	 */
	@Test
	public void assertRandomAccess9() throws Exception {
		// test case configuration
		int len = 102400;
		//int len = 2384;
		int bbSize = 64;
		int fileOffset = 14502224;
		int offset = 0;
		
		long startPlaintext = System.nanoTime();
		FileInputStream fis = new FileInputStream(plaintextFile);
		BufferedInputStream bis = new BufferedInputStream(fis);
		byte[] bufferPlaintext = new byte[len + offset];
		
		bis.mark(fileOffset + len);
		
		// skip/seek forward 
		long totalSkipped = bis.skip(fileOffset);
		while (totalSkipped < fileOffset) {
			totalSkipped += bis.skip(fileOffset - totalSkipped);
		}
		
		int plaintextByteRead = bis.read(bufferPlaintext, offset, len);
		bis.close();
		fis.close();
		long endPlaintext = System.nanoTime();
		
		StringBuilder plaintextData = new StringBuilder();
		for (int i = 0; i < plaintextByteRead; i++) {
		//for (byte b : bufferPlaintext) {
			plaintextData.append(bufferPlaintext[i] + " ");
		}
		
		
		// Access protected file
		long startEncrypted = System.nanoTime();
		TestFMSFileReaderStream fisProtected = new TestFMSFileReaderStream(
				new FileInputStream(encryptedFile));
		AbstractNextLabsDecryptingInputStream nxlDecryptingInputStream = 
				getNextLabsDecryptingInputStream(fisProtected);
		nxlDecryptingInputStream.init();
				
		ByteBuffer bufferEncrypted = ByteBuffer.allocateDirect(bbSize);
		final StringBuilder strBldrDecrypted = new StringBuilder();
		
		int encryptedByteRead = nxlDecryptingInputStream.read(
				fileOffset, bufferEncrypted, new InterfaceBufferHandler() {
					public void onBuffer(final ByteBuffer bbData, final int iBufOffset, final int iBufSize) throws Exception {
						LOGGER.trace("onBuffer() enters");
						bbData.limit(iBufOffset + iBufSize);
						bbData.position(iBufOffset);
						
						byte[] data = new byte[iBufSize];
						bbData.get(data);
						
						for (byte b : data) {
							strBldrDecrypted.append(b + " ");
						}						
						
						bbData.clear();
						LOGGER.trace("onBuffer() exits");
					}
				}, len);
		nxlDecryptingInputStream.close();
		long endEncrypted = System.nanoTime();
				
		// Test result
		LOGGER.info("9. Start [{}] => Assert random access 9 (fileoffset:{}, off:{},len:{})", 
				this.getClass().getSimpleName(), fileOffset, offset, len);
		LOGGER.info("	plaintext byteread: {}", plaintextByteRead);
		LOGGER.info("	encrypted byteread: {}", encryptedByteRead);
		LOGGER.info("	plaintext read(us): {}", (endPlaintext - startPlaintext) / 1000);
		LOGGER.info("	encrypted read(us): {}", (endEncrypted - startEncrypted) / 1000);
		if (SHOWDATA) {
			LOGGER.info("=============		PLAINTEXT DATA		============={}{}", 
					System.lineSeparator(), plaintextData.toString());
			LOGGER.info("=============		DECRYPTED DATA		============={}{}", 
					System.lineSeparator(), strBldrDecrypted.toString());
		}
		LOGGER.info("9. End => Assert random access 9");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(plaintextData.toString(), strBldrDecrypted.toString());
	}

}
