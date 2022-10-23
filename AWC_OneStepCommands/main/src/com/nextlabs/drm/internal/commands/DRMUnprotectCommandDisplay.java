/*
 * Created on March 22, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2016 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.drm.internal.commands;

import com.google.gwt.resources.client.ImageResource;
import com.nextlabs.drm.internal.Resources;
import com.nextlabs.drm.resources.i18n.DRMMessages;
import com.siemens.splm.clientfx.ui.commands.published.AbstractCommandDisplay;

public class DRMUnprotectCommandDisplay extends AbstractCommandDisplay {
	
	// Constructor
	public DRMUnprotectCommandDisplay() {
		super(DRMMessages.INSTANCE.DRMCommandTitle_unProtect());
	}

	@Override
	public ImageResource getIconResource() {
		return Resources.INSTANCE.getDRMCommandImage_unprotect();
	}
	
	
}
