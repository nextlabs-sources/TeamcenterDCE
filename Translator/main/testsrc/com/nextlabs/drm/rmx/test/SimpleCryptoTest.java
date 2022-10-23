package com.nextlabs.drm.rmx.test;

import static org.junit.Assert.assertEquals;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.junit.Test;
//import org.slf4j.Logger;
//import org.slf4j.LoggerFactory;

import com.nextlabs.drm.rmx.configuration.SimpleCrypto;

public class SimpleCryptoTest {
	
	//private final static Logger LOGGER = LoggerFactory.getLogger("TRANSLATOR_TEST_LOGGER");
	private final static Logger LOGGER = LogManager.getLogger("TRANSLATOR_TEST_LOGGER");
	private final SimpleCrypto simpleCrypto = new SimpleCrypto();
	
	@Test
	public void encrypt1() throws Exception {
		String plaintext = "password";
		String ciphertext = "CyAGUfEo4iSW+0YJRVJcjA==";
		
		// Test result
		LOGGER.info("0. Start [" + this.getClass().getSimpleName() + "] => encrypt1");
		LOGGER.info("	plaintext: " + plaintext);
		LOGGER.info("	ciphertext: " + simpleCrypto.encrypt(plaintext));
		LOGGER.info("0. End => Assert equal");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(simpleCrypto.encrypt(plaintext), ciphertext);
	}
	
	@Test
	public void encrypt2() throws Exception {
		String plaintext1 = "password";
		String plaintext2 = "password";
		
		// Test result
		LOGGER.info("1. Start [" + this.getClass().getSimpleName() + "] => encrypt2");
		LOGGER.info("	plaintext1: " + plaintext1);
		LOGGER.info("	ciphertext1: " + simpleCrypto.encrypt(plaintext1));
		LOGGER.info("   plaintext2: " + plaintext2);
		LOGGER.info("   ciphertext2: " + simpleCrypto.encrypt(plaintext2));
		LOGGER.info("1. End => Assert equal");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(simpleCrypto.encrypt(plaintext1), simpleCrypto.encrypt(plaintext2));
	}
	
	@Test
	public void encrypt3() throws Exception {
		String plaintext = "password";
		String ciphertext = simpleCrypto.encrypt(plaintext);
		
		// Test result
		LOGGER.info("2. Start [" + this.getClass().getSimpleName() + "] => encrypt3");
		LOGGER.info("   plaintext: " + plaintext);
		LOGGER.info("   ciphertext: " + ciphertext);
		LOGGER.info("   plaintext: " + simpleCrypto.decrypt(ciphertext));
		LOGGER.info("2. End => Assert equal");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(plaintext, simpleCrypto.decrypt(ciphertext));
	}
	
	@Test
	public void decrypt1() throws Exception {
		String plaintext = "password";
		String ciphertext = "CyAGUfEo4iSW+0YJRVJcjA==";
		
		// Test result
		LOGGER.info("3. Start [" + this.getClass().getSimpleName() + "] => decrypt1");
		LOGGER.info("	plaintext: " + simpleCrypto.decrypt(ciphertext));
		LOGGER.info("	ciphertext: " + ciphertext);
		LOGGER.info("3. End => Assert equal");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(plaintext, simpleCrypto.decrypt(ciphertext));
	}
	
	@Test
	public void decrypt2() throws Exception {
		String ciphertext1 = "CyAGUfEo4iSW+0YJRVJcjA==";
		String ciphertext2 = "CyAGUfEo4iSW+0YJRVJcjA==";
		
		// Test result
		LOGGER.info("4. Start [" + this.getClass().getSimpleName() + "] => decrypt2");
		LOGGER.info("	plaintext1: " + simpleCrypto.decrypt(ciphertext1));
		LOGGER.info("	ciphertext1: " + ciphertext1);
		LOGGER.info("   plaintext2: " + simpleCrypto.decrypt(ciphertext2));
		LOGGER.info("   ciphertext2: " + ciphertext2);
		LOGGER.info("4. End => Assert equal");
		LOGGER.info("");
		LOGGER.info("");
		
		// Assert
		assertEquals(simpleCrypto.decrypt(ciphertext1), simpleCrypto.decrypt(ciphertext2));
	}

}
