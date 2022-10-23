// @<COPYRIGHT>@
// ==================================================
// Copyright 2017.
// Siemens Product Lifecycle Management Software Inc.
// All Rights Reserved.
// ==================================================
// @<COPYRIGHT>@

package com.nextlabs.NXLOverlay.internal.config;

import com.gwtplatform.mvp.client.gin.AbstractPresenterModule;
import com.google.inject.Provider;
import com.nextlabs.NXLOverlay.internal.NameTokens;

import com.siemens.splm.clientfx.base.extensions.published.ExtensionsGinHelper;
import com.siemens.splm.clientfx.ui.widgets.published.utils.ICellVisualIndicator;

/**
 * Module NXLOverlay.
 */
public class NXLOverlayModule
    extends AbstractPresenterModule
{
    @Override
    protected void configure()
    {
		ExtensionsGinHelper.contributeToExtensionSet( binder(), ICellVisualIndicator.class, NXLOverlayIndicator.class );
    }

}
