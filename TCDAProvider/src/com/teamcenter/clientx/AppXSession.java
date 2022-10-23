//==================================================
// 
//  Copyright 2017 Siemens Product Lifecycle Management Software Inc. All Rights Reserved.
//
//==================================================


package com.teamcenter.clientx;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.List;
import java.util.Locale;
import java.util.Objects;
import java.util.Vector;

import com.nextlabs.dap.provider.ConnectionStatus;

import com.teamcenter.schemas.soa._2006_03.exceptions.ServiceException;

import com.teamcenter.services.strong.core._2011_06.Session.Credentials;
import com.teamcenter.services.strong.core._2011_06.Session.LoginResponse;

import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.services.strong.core.SessionService;
import com.teamcenter.soa.SoaConstants;
import com.teamcenter.soa.client.Connection;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.exceptions.NotLoadedException;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;


public class AppXSession
{
    /**
     * Single instance of the Connection object that is shared throughout
     * the application. This Connection object is needed whenever a Service
     * stub is instantiated.
     */
    private static Connection           connection;

    /**
     * The credentialManager is used both by the Session class and the Teamcenter
     * Services Framework to get user credentials.
     *
     */
    // private static AppXCredentialManager credentialManager;

    private static final Logger logger = LogManager.getLogger("TCDAPLOGGER");

    /**
     * Create an instance of the Session with a connection to the specified
     * server.
     *
     * Add implementations of the ExceptionHandler, PartialErrorListener,
     * ChangeListener, and DeleteListeners.
     *
     * @param host      Address of the host to connect to, http://serverName:port/tc
     */
    public AppXSession(String host)
    {
        // Create an instance of the CredentialManager, this is used
        // by the SOA Framework to get the user's credentials when
        // challenged by the server (session timeout on the web tier).
        AppXCredentialManager credentialManager = new AppXCredentialManager();

        String protocol=null;
        String envNameTccs = null;
        if ( host.startsWith("http") )
        {
            protocol   = SoaConstants.HTTP;
        }
        else if ( host.startsWith("tccs") )
        {
            protocol   = SoaConstants.TCCS;
            host = host.trim();
            int envNameStart = host.indexOf('/') + 2;
            envNameTccs = host.substring( envNameStart, host.length() );
            host = "";
        }
        else
        {
            System.setProperty("jacorb.suppress_no_props_warning", "on");
        }


        // Create the Connection object, no contact is made with the server
        // until a service request is made
        connection = new Connection(host,  credentialManager, SoaConstants.REST,  protocol);

        if(Objects.equals(protocol, SoaConstants.TCCS) )
        {
           connection.setOption(  Connection.TCCS_ENV_NAME, envNameTccs );
        }

        // Add an ExceptionHandler to the Connection, this will handle any
        // InternalServerException, communication errors, XML marshaling errors
        // .etc
        connection.setExceptionHandler(new AppXExceptionHandler());

        // While the above ExceptionHandler is required, all of the following
        // Listeners are optional. Client application can add as many or as few Listeners
        // of each type that they want.

        // Add a Partial Error Listener, this will be notified when ever a
        // a service returns partial errors.
        connection.getModelManager().addPartialErrorListener(new AppXPartialErrorListener());

        // Add a Change and Delete Listener, this will be notified when ever a
        // a service returns model objects that have been updated or deleted.
        connection.getModelManager().addModelEventListener(new AppXModelEventListener());

    }

    /**
     * Get the single Connection object for the application
     *
     * @return  connection
     */
    public static Connection getConnection()
    {
        return connection;
    }

    /**
     * Login to the Teamcenter Server
     *
     */
    public void login(String proxyUsername, String proxyUserPassword) throws Exception {
        // Get the service stub
        SessionService sessionService = SessionService.getService(connection);

        Credentials cred = new Credentials();
        cred.user = proxyUsername;
        cred.password = proxyUserPassword;
        LoginResponse out = sessionService.login(cred);
    }

    /**
     * Get the current status of Connection object (still connected to retrieve information
     */
    public ConnectionStatus connectionStatus() {
        String locale = connection.getLocale();

        if (locale == null || locale.isEmpty()) {
            return new ConnectionStatus.Builder(false).message("Connection FAILED").build();
        } else {
            return new ConnectionStatus.Builder(true).build();
        }
    }

    /**
     * Terminate the session with the Teamcenter Server
     *
     */
    public void logout()
    {
        // Get the service stub
        SessionService sessionService = SessionService.getService(connection);
        try
        {
            sessionService.logout();
        }
        catch (ServiceException e){}

        if (connection == null) {
            System.out.println("NULL connection object");
        }
    }

    /**
     * Print some basic information for a list of objects
     *
     * @param objects
     */
    public static void printObjects(ModelObject[] objects)
    {
        if(objects == null)
            return;

        SimpleDateFormat format = new SimpleDateFormat("M/d/yyyy h:mm a", new Locale("en", "US")); // Simple no time zone

        // Ensure that the referenced User objects that we will use below are loaded
        getUsers( objects );

        System.out.println("Name\t\tOwner\t\tLast Modified");
        System.out.println("====\t\t=====\t\t=============");
        for (int i = 0; i < objects.length; i++)
        {
            if (!objects[i].getTypeObject().isInstanceOf("WorkspaceObject"))
                continue;

            ModelObject wo = objects[i];
            try
            {
                String name = wo.getPropertyObject("object_string").getStringValue();
                ModelObject owner = wo.getPropertyObject("owning_user").getModelObjectValue();
                Calendar lastModified =wo.getPropertyObject("last_mod_date").getCalendarValue();

                System.out.println(name + "\t" + owner.getPropertyObject("user_name").getStringValue() + "\t"
                        + format.format(lastModified.getTime()));
            }
            catch (NotLoadedException e)
            {
                // Print out a message, and skip to the next item in the folder
                // Could do a DataManagementService.getProperties call at this point
                System.out.println(e.getMessage());
                System.out.println("The Object Property Policy ($TC_DATA/soa/policies/Default.xml) is not configured with this property.");
            }
        }

    }


    private static void getUsers( ModelObject[] objects )
    {
        if(objects == null)
            return;

        DataManagementService dmService = DataManagementService.getService(AppXSession.getConnection());

        List<ModelObject> unKnownUsers = new Vector<ModelObject>();
        for (int i = 0; i < objects.length; i++)
        {
            if (!objects[i].getTypeObject().isInstanceOf("WorkspaceObject"))
                continue;

            ModelObject wo = objects[i];

            ModelObject owner = null;
            try
            {
                owner = wo.getPropertyObject("owning_user").getModelObjectValue();
                owner.getPropertyObject("user_name");
            }
            catch (NotLoadedException e)
            {
                if(owner != null)
                    unKnownUsers.add(owner);
            }
        }
        ModelObject[] users = (ModelObject[])unKnownUsers.toArray(new ModelObject[unKnownUsers.size()]);
        String[] attributes = { "user_name" };


        // *****************************
        // Execute the service operation
        // *****************************
        dmService.getProperties(users, attributes);


    }


}
