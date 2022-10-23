package com.nextlabs.swrmx.nxlutils;
import com.nextlabs.swrmx.socket.SwRMXClientSocket;
import com.nextlabs.swrmx.socket.SwRMXServerSocket;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.util.logging.Logger;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

import com.nextlabs.swrmx.logger.SwRMXSwimLogger;
import com.nextlabs.swrmx.nxlutils.SwRMXResult;
import com.nextlabs.swrmx.nxlutils.SwRMXStatus;
import com.nextlabs.swrmx.nxlutils.OperationType;

public class NxlUtils {
 	public static boolean isSldWorksRMXRunning = false;
	private static final String hostName = "localhost";
	private static final int portNum = 27215;
	static final SwRMXSwimLogger logger;
	static SwRMXClientSocket rmxClientSocket;
	public static int connStat ;
	
	static{
		logger =  SwRMXSwimLogger.getInstance();
				
		
		//Start server socket in separate thread
		Thread th = new Thread(new SwRMXServerSocket()); 
        th.start();
		
        //Start the client socket
		rmxClientSocket=new SwRMXClientSocket(hostName, portNum);
		connStat = rmxClientSocket.init();	
		if(connStat != SwRMXStatus.SOCKET_INIT_SUCCESS.getValue()){
			//ShowMessageDialog(SwRMXStatus.SOCKET_CONNECTION_NOT_ESTABLISHED.getValue(),"");
			logger.error("Socket connection cannot be established." + "HostName = " + hostName + "," +" Port Number =" + Integer.toString(portNum) +" Failed Reasons : " +connStat);
		}
		else{
			isSldWorksRMXRunning = true;
			logger.info("Socket connection successfully established." + "HostName = " + hostName + "," +" Port Number =" + Integer.toString(portNum));
		}
	}
	
	
	
	public NxlUtils() {
		logger.info("NxlUtils Constructor");
	}
	
	public static boolean initialize() {
		logger.info("NxlUtils initializing...");
		return true;
	}
	
	
	public static boolean nxlPerformAccessCheck(String fileName) throws Exception {
		
		boolean authorizeStatus = true;
		if(!isSldWorksRMXRunning) {
			return true;
		}
		File file = new File(fileName);
        if (file.exists()) {
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            DocumentBuilder db = dbf.newDocumentBuilder();
            Document dom = db.parse(file);
            Element docEle = dom.getDocumentElement();
            String action = docEle.getAttribute("action");
			String operation = docEle.getAttribute("operation");
			logger.info("Action :" +action + " Operation: " + operation);
			authorizeStatus = nxlAuthorizationCheck(action,operation,fileName);
			if(!authorizeStatus) {
				logger.error("User not authorized to perform " + "Action :" +action + "," +" Operation: " + operation);
			}
        }
        else {
        	logger.error("File doesn't exists : " +fileName);
        }
        return authorizeStatus;
	}
	
	public static boolean nxlProcessResponseFile(String fileName) throws Exception {
		boolean status = false;
		if(!isSldWorksRMXRunning) {
			return true;
		}
		String action = "";
		File file = new File(fileName);
        if (file.exists()) {
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            DocumentBuilder db = dbf.newDocumentBuilder();
            Document dom = db.parse(file);
            Element docEle = dom.getDocumentElement();
            action = docEle.getAttribute("action");
			logger.info("Action: " + action);	
        }
        else {
        	logger.error("File doesn't exists : " +fileName);
        }
        
        if(action != null && !action.isEmpty()) {
        	  SwRMXResult responseResult;		
              //Make call to SolidWorks RMX again in order to enable it to make a map of source->destination files
                if(action.equalsIgnoreCase("new")) {
                	responseResult = sendOperationResponse(OperationType.PROCESS_SAVE_NEW.getValueString(),fileName);
                	status = responseResult.getOperationStatus();
                	if(!status)
                		logger.error("Error occured while processing the response xml file for action : " +action);
                }
                else if (action.equalsIgnoreCase("replace")){
                	responseResult = sendOperationResponse(OperationType.PROCESS_SAVE_REPLACE.getValueString(),fileName);
                	status = responseResult.getOperationStatus();
                	if(!status)
                		logger.error("Error occured while processing the response xml file for action : " +action);
                }
                else {
                	logger.warn("Not a supported action type. Action : " +action);
                	status = true;
                }
        }
          	
    	return status;
		
	}
	
	public static boolean nxlAuthorizationCheck(String action,String operation,String fileName) {
		boolean authorizeStatus = true;
		SwRMXResult opResult;
		if(action.equalsIgnoreCase("validation")) {
			if(operation != null && !operation.isEmpty()) {
				if(operation.equalsIgnoreCase("Save")){
					//User chose 'Save only cheked out and new model to teamcenter'
					//attrVal = 'Save'
						opResult = checkUserAuthorization(OperationType.SAVE.getValueString(),fileName);
					}
					else if(operation.equalsIgnoreCase("Save All")){
					//User chose 'Save All'
					//attrVal = 'Save All''
						opResult = checkUserAuthorization(OperationType.SAVE_ALL.getValueString(),fileName);					
					}
					else if(operation.contains("Save As")){
					//User chose 'Save Current Model to Teamcenter' or 'Save As' or 'Save As Copy'
					//attrVal = 'Save As'  when dialog is launched through 'Save Current Model to teamcenter'
					//attrVal = 'RMB->Save As' when right click and select 'Save As'
					//attrVal = 'RMB->Save As Copy' when right click and select 'Save As Copy'
						opResult = checkUserAuthorization(OperationType.SAVE_AS.getValueString(),fileName);	
						
					}
					else if(operation.equalsIgnoreCase("Clone")){
						opResult = checkUserAuthorization(OperationType.CLONE.getValueString(),fileName);	
					}
					else{
						opResult = new SwRMXResult();
						opResult.setOperationStatus(true);
						authorizeStatus = true;
						logger.warn("Not a supported operation type. Action : " +action + " Operation : " +operation +" Setting authorizeStatus to true.");
					}
						
					if(!opResult.getOperationStatus()){
						authorizeStatus = false;
						opResult.showErrorMessage();
					}							
			}	
		}
		else if(action.equalsIgnoreCase("new")) {
			opResult = checkUserAuthorization(OperationType.SAVE_NEW.getValueString(),fileName);
			if(!opResult.getOperationStatus()){
				authorizeStatus = false;
				opResult.showErrorMessage();
			}
		}
		else if(action.equalsIgnoreCase("replace")) {
			opResult = checkUserAuthorization(OperationType.SAVE_REPLACE.getValueString(),fileName);
			if(!opResult.getOperationStatus()){
				authorizeStatus = false;
				opResult.showErrorMessage();
			}
			
		}
		else {
			opResult = new SwRMXResult();
			opResult.setOperationStatus(true);
			authorizeStatus = true;
			logger.warn("Not a supported action type. Action : " +action + " Operation : " +operation + " Setting authorizeStatus to true.");
		}
		return authorizeStatus;
	}
			
	static SwRMXResult  checkUserAuthorization(String opType,String fileName) {
		logger.info("OperationType :" + opType);
		SwRMXResult opResult;
		if(connStat != SwRMXStatus.SOCKET_INIT_SUCCESS.getValue()){
			logger.info("Previous socket connection not valid . Trying to connect again...");
			connStat = rmxClientSocket.init();
			if(connStat != SwRMXStatus.SOCKET_INIT_SUCCESS.getValue()){
				logger.error("Attempt to re-establish socket connection failed.");
				opResult = new SwRMXResult();
				opResult.setOperationStatus(false);
				opResult.setError(SwRMXStatus.SOCKET_CONNECTION_NOT_ESTABLISHED.getValue());
				return opResult;
			}
		}
		opResult=rmxClientSocket.sendDataToRMXServerSocket(opType+fileName);
		if(opResult.getError() == SwRMXStatus.SOCKET_CONN_RESET.getValue()){
			logger.error("Socket connection reset. Possibly there is no server socket listening for the incoming client request. Please check the connection.");
			opResult.setOperationStatus(false);
			return opResult;
		}
		
		logger.info("Operation : " +opType + "," +" Data received from server : " +opResult.getReadData());
		int errStatus = Integer.parseInt(String.valueOf(opResult.getReadData().charAt(0)));
		if(errStatus != SwUserAuthErrReasons.TC_SUCCESS.getValue()){
			opResult.setOperationType(Integer.parseInt(opType));
			opResult.setOperationStatus(false);
			opResult.setError(SwRMXStatus.USER_NOT_AUTHORISED.getValue());
			opResult.setErrorReason(errStatus);
		}
		else{
				opResult.setOperationStatus(true);
		}
		return opResult;
	}
	
	
	public static SwRMXResult  sendOperationResponse(String opType,String fileName) {
		logger.info("OperationType :" + opType + "," +" FileName : " + fileName);
		SwRMXResult opResult;
		if(connStat != SwRMXStatus.SOCKET_INIT_SUCCESS.getValue()){
			logger.info("Previous socket connection not valid . Trying to connect again...");
			connStat = rmxClientSocket.init();
			if(connStat != SwRMXStatus.SOCKET_INIT_SUCCESS.getValue()){
				logger.error("Attempt to re-establish socket connection failed.");
				opResult = new SwRMXResult();
				opResult.setOperationStatus(false);
				opResult.setError(SwRMXStatus.SOCKET_CONNECTION_NOT_ESTABLISHED.getValue());
				return opResult;
			}
		}
		opResult=rmxClientSocket.sendDataToRMXServerSocket(opType+fileName);
		if(opResult.getError() == SwRMXStatus.SOCKET_CONN_RESET.getValue()){
			logger.error("Socket connection reset. Possibly there is no server socket listening for the incoming client request. Please check the connection.");
			opResult.setOperationStatus(false);
			return opResult;
		}
		
		logger.info("Operation : " +opType + "," +" Data received from server : " +opResult.getReadData());
		int errStatus = Integer.parseInt(String.valueOf(opResult.getReadData().charAt(0)));
		if(errStatus != SwUserAuthErrReasons.TC_SUCCESS.getValue()){
			logger.error("Error occured while processing the response xml on the server.");
			opResult.setOperationStatus(false);
		}
		else {
			opResult.setOperationStatus(true);
		}
		
		return opResult;
	}
	

}
