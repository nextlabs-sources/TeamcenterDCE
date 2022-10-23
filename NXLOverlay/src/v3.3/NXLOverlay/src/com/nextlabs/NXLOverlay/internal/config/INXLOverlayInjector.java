// @<COPYRIGHT>@
// ==================================================
// Copyright 2017.
// Siemens Product Lifecycle Management Software Inc.
// All Rights Reserved.
// ==================================================
// @<COPYRIGHT>@

package com.nextlabs.NXLOverlay.internal.config;

import com.google.gwt.core.client.GWT;
import com.google.gwt.inject.client.GinModules;
import com.siemens.splm.clientfx.base.extensions.published.IModuleInjector;
import com.nextlabs.NXLOverlay.internal.config.NXLOverlayModule;

/**
 * access functions to injected types.  This is limited to the module level access as 
 * we are in the internal namespace.
 */
@GinModules( NXLOverlayModule.class )
public interface INXLOverlayInjector
    extends IModuleInjector
{
    /** Injector instance */
    INXLOverlayInjector INSTANCE = GWT.create( INXLOverlayInjector.class );

}
