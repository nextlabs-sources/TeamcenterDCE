package com.nextlabs.dap.provider.exception;

public class InvalidUserIdException extends Exception {
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 4892406384247207396L;

	public InvalidUserIdException(String userId) {
		super("invalid user Id: " + userId);
	}

}
