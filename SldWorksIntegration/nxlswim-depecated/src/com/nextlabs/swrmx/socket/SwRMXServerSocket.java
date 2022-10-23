package com.nextlabs.swrmx.socket;

import java.net.Socket;
import java.net.ServerSocket;
import java.io.StringWriter;
import java.io.PrintWriter;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.UnknownHostException;
import java.util.logging.Logger;
import java.io.IOException;
import com.nextlabs.swrmx.logger.SwRMXSwimLogger;
import com.nextlabs.swrmx.nxlutils.NxlUtils;
import com.nextlabs.swrmx.nxlutils.SwRMXStatus;



public class SwRMXServerSocket implements Runnable {
	public static final int DEFAULT_PORT = 27216	;
	public static final SwRMXSwimLogger logger = SwRMXSwimLogger.getInstance();
	private ServerSocket serverSocket;
	//private Socket clientSocket;
	StringWriter sw = new StringWriter();
	PrintWriter pw = new PrintWriter(sw);
	
	public SwRMXServerSocket(){
		logger.info("SwRMXServerSocket constructor");
	}

	public void run() {

    	try {	
             serverSocket = new ServerSocket(DEFAULT_PORT);
    	}
    	catch(IOException e) {
    		logger.error("I/O error occured when opening the server socket on "  + String.valueOf(DEFAULT_PORT) );
    		e.printStackTrace(pw);
    		logger.error(sw.toString());
    	}
    	catch(SecurityException e) {
    		logger.error("Security manager exists and its checkListen method doesn't allow the operation on  "  + String.valueOf(DEFAULT_PORT) );
    		e.printStackTrace(pw);
    		logger.error(sw.toString());
    		
    	}
    	catch(IllegalArgumentException e) {
    		logger.error("Port parameter is outside the specified range of valid port values, which is between 0 and 65535, inclusive. Used Port number = " + String.valueOf(DEFAULT_PORT));
    		
    	}
    	
    	try {
    		while(true) {
    			Socket clientSocket = serverSocket.accept();
    			logger.info("New client connection made");
    			new Thread(new ServerThread(clientSocket)).start();
    		}
    	}
    	catch(IOException e){
    		logger.error("I/O error occurs when waiting for a connection");
    		e.printStackTrace(pw);
    		logger.error(sw.toString());
    		
    	}
    	catch(SecurityException e) {
    		logger.error("Security manager exists and its checkAccept method doesn't allow the operation" );
    		e.printStackTrace(pw);
    		logger.error(sw.toString());
    		
    	}
    	
    	logger.info("Exiting run method.");

	}
}

class ServerThread implements Runnable {
    private Socket socket;
    public static final SwRMXSwimLogger logger = SwRMXSwimLogger.getInstance();
	private PrintWriter out;
	private BufferedReader in;
    StringWriter sw = new StringWriter();
	PrintWriter pw = new PrintWriter(sw);
 
    public ServerThread(Socket socket) {
    	logger.info("ServerThread Constructor");
        this.socket = socket;
    }
 
    public void run() {
        try {
        	
        	out = new PrintWriter(socket.getOutputStream(), true);
   	     	in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        	
 
   	     	String inputLine, outputLine;
 
   	     	while ((inputLine = in.readLine()) != null) {
   	     		logger.info(inputLine);
   	     		if (inputLine.indexOf("Bye") != -1) {
   	     		    NxlUtils.isSldWorksRMXRunning = false;
   	     		    NxlUtils.connStat = SwRMXStatus.SOCKET_CONN_RESET.getValue() ;
   	     			logger.info("SolidWorks RMX Exiting. Setting flag isSldWorksRMXRunning to false.");
   	     		}
   	     		else if(inputLine.indexOf("Hello") != -1) {
   	     		    NxlUtils.isSldWorksRMXRunning = true;
   	     			logger.info("Solidworks RMX loading again. Setting flag isSldWorksRMXRunning to true.");
   	     		}
   	     		out.println("Ack");
   	     	
   	     	}
 
            socket.close();
        } catch (IOException e) {
        	logger.error("Exception caught when trying to read or write on port");
    		e.printStackTrace(pw);
    		logger.error(sw.toString());
        }
        logger.info("Exiting run method");
    }
}


