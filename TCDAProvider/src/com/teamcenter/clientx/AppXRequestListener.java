//==================================================
// 
//  Copyright 2017 Siemens Product Lifecycle Management Software Inc. All Rights Reserved.
//
//==================================================

package com.teamcenter.clientx;

import com.teamcenter.soa.client.RequestListener;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * This implementation of the RequestListener, logs each service request
 * to the console.
 *
 */
public class AppXRequestListener implements RequestListener
{
    private static final Logger logger = LogManager.getLogger("TCDAPLOGGER");

    /**
     * Called before each request is sent to the server.
     */
    public void serviceRequest ( final Info info )
    {
         // will log the service name when done
    }
    
    /**
     * Called after each response from the server.
     * Log the service operation to the console.
     */
    public void serviceResponse( final Info info )
    {
        logger.debug(() -> info.id +": "+info.service+"."+info.operation);
    }

}
