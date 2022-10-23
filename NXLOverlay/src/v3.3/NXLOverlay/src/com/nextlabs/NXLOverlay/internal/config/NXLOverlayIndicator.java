// @<COPYRIGHT>@
// ==================================================
// Copyright 2016.
// Siemens Product Lifecycle Management Software Inc.
// All Rights Reserved.
// ==================================================
// @<COPYRIGHT>@

package com.nextlabs.NXLOverlay.internal.config;

import com.google.gwt.resources.client.ImageResource;
import com.google.gwt.safehtml.shared.SafeHtml;
import com.siemens.splm.clientfx.kernel.clientmodel.published.IModelObject;
import com.siemens.splm.clientfx.kernel.clientmodel.published.IProperty;
import com.siemens.splm.clientfx.kernel.published.IPropertyName;
import com.siemens.splm.clientfx.kernel.published.ITypeName;
import com.siemens.splm.clientfx.ui.published.IIconService;
import com.siemens.splm.clientfx.ui.published.config.IUIInjector;
import com.siemens.splm.clientfx.ui.published.utils.ISvgConstants;
import com.siemens.splm.clientfx.ui.widgets.published.utils.ICellVisualIndicator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Contributor for NXLOverlay indicator
 */
public class NXLOverlayIndicator
    implements ICellVisualIndicator
{

    /** Priority for NXLOverlayIndicator. */
    private static final int NXLOVERLAY_IND_PRIORITY = 500;

    @Override
    public SafeHtml getSvgSafeHtml( IModelObject modelObject )
    {
        List<String> tooltipList = getTooltip( modelObject );
        if( tooltipList != null && !tooltipList.isEmpty() )
        {
            IIconService iconSvc = IUIInjector.INSTANCE.getIconService();
            if( iconSvc != null )
            {
                ImageResource imgRes = iconSvc.getIndicatorImageResource( "NXLOverlay" );

                if( imgRes == null )
                {
                    imgRes = iconSvc.getMissingImageResource();
                }
                return iconSvc.toHTML( imgRes );
            }
        }
        return null;
    }

    @Override
    public List<String> getTooltip( IModelObject modelObject )
    {
        if( modelObject != null && modelObject.getPropertyObject("awp0CellProperties") != null)
        {
			List<String> properties = modelObject.getPropertyObject("awp0CellProperties").getDisplayValues();
			for(String property : properties) {
				if(property.contains("Tool Used\\:Nxl3_FileHandler")) {
					
					return properties;
				}
			}            
        }
        return null;
}

    @Override
    public Integer getPriority()
    {
        return Integer.valueOf( NXLOverlayIndicator.NXLOVERLAY_IND_PRIORITY );
    }

    @Override
    public Map<String, String[]> getPropertiesToLoad()
    {
        Map<String, String[]> typeToPropertyMap = new HashMap<>();
        typeToPropertyMap.put( ITypeName.WorkspaceObject, new String[] { IPropertyName.OBJECT_NAME } );

        return typeToPropertyMap;
    }
}
