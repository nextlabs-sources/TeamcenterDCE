/*
 * Created on March 22, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2016 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.drm.internal;

import com.google.gwt.core.client.GWT;
import com.google.gwt.resources.client.ClientBundle;
import com.google.gwt.resources.client.ImageResource;

/**
 * Module resources for DRMModule
 */
public interface Resources extends ClientBundle {
	// Resources instance
	Resources INSTANCE = GWT.create(Resources.class);

	/**
	 * Get DRMCommand command image
	 * 
	 * @return DRMCommand command image
	 */
	@Source("com/nextlabs/drm/resources/images/DRMCommand.png")
	ImageResource getDRMCommandImage();

	@Source("com/nextlabs/drm/resources/images/DRMCommand_unprotect.png")
	ImageResource getDRMCommandImage_unprotect();
}
