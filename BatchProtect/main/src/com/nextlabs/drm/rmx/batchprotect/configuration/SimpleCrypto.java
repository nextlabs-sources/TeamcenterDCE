package com.nextlabs.drm.rmx.batchprotect.configuration;

import java.security.SecureRandom;
import java.util.Arrays;

/**
 * Created on November 27, 2015
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

// will be improved to store the encryption key in java key store

import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import jakarta.xml.bind.DatatypeConverter;

public class SimpleCrypto {
	
	private static final String KEY = "DCE1234!NeXTlAbS"; 			// 128 bit key
	private static final SecureRandom random = new SecureRandom();
	
	private static final String ALGORITHM = "AES";
	private static final String CHAR_SET = "UTF-8";
	private static final String TRANSFORMATION = "AES/CBC/PKCS5PADDING";
	
	public static final String PREFIX_CRYPT = "crypt:";
	
	public String encrypt(String value) throws Exception {
		// 16 bytes random IV
		byte[] bytesIV = new byte[16];
		random.nextBytes(bytesIV);
		
		IvParameterSpec iv = new IvParameterSpec(bytesIV);
        SecretKeySpec skeySpec = new SecretKeySpec(KEY.getBytes(CHAR_SET), ALGORITHM);

        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        cipher.init(Cipher.ENCRYPT_MODE, skeySpec, iv);

        byte[] encrypted = cipher.doFinal(value.getBytes());
        byte[] ivencrypted = new byte[bytesIV.length + encrypted.length];
        System.arraycopy(bytesIV, 0, ivencrypted, 0, bytesIV.length);
        System.arraycopy(encrypted, 0, ivencrypted, bytesIV.length, encrypted.length);

        return DatatypeConverter.printBase64Binary(ivencrypted);
    }

    public String decrypt(String encrypted) throws Exception {
    	byte[] ivencrypted = DatatypeConverter.parseBase64Binary(encrypted);
    	// 16 bytes IV
		byte[] bytesIV = Arrays.copyOfRange(ivencrypted, 0, 16);
		
		IvParameterSpec iv = new IvParameterSpec(bytesIV);
        SecretKeySpec skeySpec = new SecretKeySpec(KEY.getBytes(CHAR_SET), ALGORITHM);

        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        cipher.init(Cipher.DECRYPT_MODE, skeySpec, iv);

        byte[] original = cipher.doFinal(
        		Arrays.copyOfRange(ivencrypted, 16, ivencrypted.length));

        return new String(original);
    }
    
}
