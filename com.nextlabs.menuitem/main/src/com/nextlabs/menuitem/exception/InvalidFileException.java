package com.nextlabs.menuitem.exception;

/**
 * Created on Aug 15, 2015
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2017 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

public class InvalidFileException extends Exception {

	private static final long serialVersionUID = -4107494587131875658L;
	private String dsName;
	
	public InvalidFileException(String dsName) {
		super();
		this.dsName = dsName;
	}
	
	public InvalidFileException() {
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
