package com.nextlabs.cadrmx.sw;

import java.net.Socket;
import java.net.ServerSocket;
import java.io.StringWriter;
import java.io.PrintWriter;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.UnknownHostException;
import java.util.logging.Logger;
import java.io.IOException;

import com.nextlabs.cadrmx.util.RMXLogHolder;
import com.nextlabs.cadrmx.sw.SwRMXStatus;

public class SwRMXServerSocket implements Runnable {
	public static final int DEFAULT_PORT = 27216	;
	private ServerSocket _serverSocket;
	//private Socket clientSocket;
	StringWriter _sw = new StringWriter();
	PrintWriter _pw = new PrintWriter(_sw);
	
	public SwRMXServerSocket(){
		RMXLogHolder.info("SwRMXServerSocket constructor");
	}

	public void run() {

    	try {	
             _serverSocket = new ServerSocket(DEFAULT_PORT);
    	}
    	catch(IOException e) {
    		RMXLogHolder.error("I/O error occured when opening the server socket on "  + String.valueOf(DEFAULT_PORT) );
    		e.printStackTrace(_pw);
    		RMXLogHolder.error(_sw.toString());
    	}
    	catch(SecurityException e) {
    		RMXLogHolder.error("Security manager exists and its checkListen method doesn't allow the operation on  "  + String.valueOf(DEFAULT_PORT) );
    		e.printStackTrace(_pw);
    		RMXLogHolder.error(_sw.toString());
    		
    	}
    	catch(IllegalArgumentException e) {
    		RMXLogHolder.error("Port parameter is outside the specified range of valid port values, which is between 0 and 65535, inclusive. Used Port number = " + String.valueOf(DEFAULT_PORT));
    		
    	}
    	
    	try {
    		while(true) {
    			Socket clientSocket = _serverSocket.accept();
    			RMXLogHolder.info("New client connection made");
    			new Thread(new ServerThread(clientSocket)).start();
    		}
    	}
    	catch(IOException e){
    		RMXLogHolder.error("I/O error occurs when waiting for a connection");
    		e.printStackTrace(_pw);
    		RMXLogHolder.error(_sw.toString());
    		
    	}
    	catch(SecurityException e) {
    		RMXLogHolder.error("Security manager exists and its checkAccept method doesn't allow the operation" );
    		e.printStackTrace(_pw);
    		RMXLogHolder.error(_sw.toString());
    		
    	}
    	
    	RMXLogHolder.info("Exiting run method.");

	}
}

class ServerThread implements Runnable {
    private Socket _socket;
	private PrintWriter _out;
	private BufferedReader _in;
    StringWriter _sw = new StringWriter();
	PrintWriter _pw = new PrintWriter(_sw);
 
    public ServerThread(Socket socket) {
    	RMXLogHolder.info("ServerThread Constructor");
        this._socket = socket;
    }
 
    public void run() {
        try {
        	
        	_out = new PrintWriter(_socket.getOutputStream(), true);
   	     	_in = new BufferedReader(new InputStreamReader(_socket.getInputStream()));
        	
 
   	     	String inputLine, outputLine;
 
   	     	while ((inputLine = _in.readLine()) != null) {
   	     		RMXLogHolder.info(inputLine);
   	     		if (inputLine.indexOf("Bye") != -1) {
   	     		    SwimRMXSession.notifyRMXRunning(false);
   	     		    RMXLogHolder.info("SolidWorks RMX Exiting. Notifying RMX not running.");
   	     		}
   	     		else if(inputLine.indexOf("Hello") != -1) {
   	     			SwimRMXSession.notifyRMXRunning(true);
   	     		    RMXLogHolder.info("Solidworks RMX loading again. Notifying RMX is running.");
   	     		}
   	     		_out.println("Ack");
   	     	
   	     	}
 
            _socket.close();
        } catch (IOException e) {
        	RMXLogHolder.error("Exception caught when trying to read or write on port");
    		e.printStackTrace(_pw);
    		RMXLogHolder.error(_sw.toString());
        }
        RMXLogHolder.info("Exiting run method");
    }
}


