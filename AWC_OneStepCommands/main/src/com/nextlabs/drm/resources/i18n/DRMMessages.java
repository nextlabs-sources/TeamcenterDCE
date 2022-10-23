/*
 * Created on March 22, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2016 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.drm.resources.i18n;

import com.google.gwt.core.client.GWT;
import com.google.gwt.i18n.client.Messages;

/**
 * Messages for DRMModule.
 */
public interface DRMMessages extends Messages {
	
	/** Messages instance */
	DRMMessages INSTANCE = GWT.create(DRMMessages.class);

	/**
	 * Provides the translated SampleCommand command title.
	 * 
	 * @return translated SampleCommand command title
	 */
	String DRMCommandTitle();
	String DRMCommandTitle_unProtect();

}
