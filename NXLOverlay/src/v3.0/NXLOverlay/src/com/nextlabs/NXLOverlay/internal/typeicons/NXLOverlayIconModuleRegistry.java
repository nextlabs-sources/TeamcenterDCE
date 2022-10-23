// @<COPYRIGHT>@
// ==================================================
// Copyright 2016.
// Siemens Product Lifecycle Management Software Inc.
// All Rights Reserved.
// ==================================================
// @<COPYRIGHT>@

package com.nextlabs.NXLOverlay.internal.typeicons;

import java.util.Arrays;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

import com.google.gwt.resources.client.ImageResource;
import com.google.gwt.safehtml.shared.SafeHtml;
import com.google.gwt.safehtml.shared.SafeHtmlUtils;
import com.google.gwt.safehtml.shared.SafeUri;
import com.siemens.splm.clientfx.kernel.clientmodel.published.IModelObject;
import com.siemens.splm.clientfx.kernel.clientmodel.published.IProperty;
import com.siemens.splm.clientfx.kernel.published.ITypeName;
import com.siemens.splm.clientfx.tcui.typeicons.published.ITypeIconModuleRegistry;
import com.siemens.splm.clientfx.tcui.utils.published.ModelUtils;
import com.siemens.splm.clientfx.tcui.xrt.internal.views.ShowObjectPrimaryWorkAreaView;
import com.nextlabs.NXLOverlay.internal.Resources;

/**
 * Assign type icons to a type
 */
public class NXLOverlayIconModuleRegistry
    implements ITypeIconModuleRegistry
{
    @Override
    public ImageResource getTypeIcon( String typeName ) 
    {
        // Assign the NXLIconImageIcon.png to the data type NXLIcon
        if( typeName.equals( "NXLIcon" ) ) //$NON-NLS-1$
        {
            return Resources.INSTANCE.getNXLIconIconItem();
        }
        return null;
    }

    @Override
    public SafeHtml getCustomIcon( IModelObject object )
    {
        return null;
    }

    @Override
    public SafeUri getThumbnailURL( IModelObject object )
    {
		if(object.getPropertyObject("awp0CellProperties") == null) return null;
    	
    	List<String> properties = object.getPropertyObject("awp0CellProperties").getDisplayValues();
    	for(String property : properties) {
    		if(property.contains("Nxl3_FileHandler")) {
    			
    			return Resources.INSTANCE.getNXLIconIconItem().getSafeUri();
    		}
    	}
        return null;
    }

    @Override
    public SafeHtml getMiniOverlayIconSafeHtml( IModelObject object )
    {
        return null;
    }

    @Override
    public SafeUri getThumbnailOverlayURI( IModelObject object )
    {
        return null;
    }
}
