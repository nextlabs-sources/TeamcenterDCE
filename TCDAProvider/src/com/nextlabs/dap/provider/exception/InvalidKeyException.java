package com.nextlabs.dap.provider.exception;

public class InvalidKeyException extends Exception {
	
	/**
	 * 
	 */
	private static final long serialVersionUID = -9072991082187420687L;

	public InvalidKeyException(String key) {
		super("invalid key: " + key);
	}

}
