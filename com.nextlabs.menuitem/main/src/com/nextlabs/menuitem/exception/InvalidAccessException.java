package com.nextlabs.menuitem.exception;

/**
 * Created on Oct 9, 2017
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2017 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

public class InvalidAccessException extends Exception {

	private static final long serialVersionUID = -7105072916081193799L;
	private String dsName;
	
	public InvalidAccessException(String dsName) {
		super();
		this.dsName = dsName;
	}
	
	public InvalidAccessException() {
		super();
		this.dsName = "";
	}
	
	/**
	 * Returns the name of a dataset that does not have any file reference
	 * 
	 * @return dataset's name
	 */
	public String getDsName() {
		return dsName;
	}
	
}
