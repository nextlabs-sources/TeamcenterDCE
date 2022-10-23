// @<COPYRIGHT>@
// ==================================================
// Copyright 2016.
// Siemens Product Lifecycle Management Software Inc.
// All Rights Reserved.
// ==================================================
// @<COPYRIGHT>@

package com.nextlabs.NXLOverlay.internal.config;

import com.nextlabs.NXLOverlay.internal.typeicons.NXLOverlayIconModuleRegistry;
import com.siemens.splm.clientfx.tcui.typeicons.published.TypeIconExtensionPointHelper;
import com.gwtplatform.mvp.client.gin.AbstractPresenterModule;
import com.google.inject.Provider;
import com.nextlabs.NXLOverlay.internal.NameTokens;

/**
 * Module NXLOverlayIcon.
 */
public class NXLOverlayIconModule
    extends AbstractPresenterModule
{
    @Override
    protected void configure()
    {

        //Contribute the NXLOverlayIconModuleRegistry class to the TypeIconExtensionPointHelper.
        //This must be done anywhere that type icons are assigned
        TypeIconExtensionPointHelper.contributeTypeIconModuleRegistry( binder(),NXLOverlayIconModuleRegistry.class );
    }
}
