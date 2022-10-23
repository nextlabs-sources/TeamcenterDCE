// @<COPYRIGHT>@
// ==================================================
// Copyright 2016.
// Siemens Product Lifecycle Management Software Inc.
// All Rights Reserved.
// ==================================================
// @<COPYRIGHT>@

package com.nextlabs.NXLOverlay.internal;

import com.google.gwt.core.client.GWT;
import com.google.gwt.resources.client.ClientBundle;
import com.google.gwt.resources.client.ImageResource;

/**
 * Module resources for NXLOverlayIcon
 */
public interface Resources
    extends ClientBundle
{
    /** Resources instance */
    Resources INSTANCE = GWT.create( Resources.class );

    /** Returns the sample type icon image.
     * @return ImageResource The sample type icon image.
     */
@Source( "com/nextlabs/NXLOverlay/resources/images/NXLIconIconImage.png" )
ImageResource getNXLIconIconItem();

}
