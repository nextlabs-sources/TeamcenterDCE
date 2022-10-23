/*
 * Created on March 22, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2016 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.drm.internal.config;

import com.google.gwt.core.client.GWT;
import com.google.gwt.inject.client.GinModules;
import com.nextlabs.drm.internal.config.DRMModule;
import com.siemens.splm.clientfx.base.extensions.published.IModuleInjector;

/**
 * access functions to injected types. This is limited to the module level
 * access as we are in the internal namespace.
 */
@GinModules(DRMModule.class)
public interface IDRMInjector extends IModuleInjector {

	// Injector instance
	IDRMInjector INSTANCE = GWT.create(IDRMInjector.class);

}
