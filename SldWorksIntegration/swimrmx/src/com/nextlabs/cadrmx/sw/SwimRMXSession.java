package com.nextlabs.cadrmx.sw;

// Java system
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.nio.file.Paths;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.UIManager;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

// SwimRMX
import com.nextlabs.cadrmx.util.RMXLogHolder;
import com.nextlabs.cadrmx.sw.OperationType;
import com.nextlabs.cadrmx.sw.SwRMXClientSocket;
import com.nextlabs.cadrmx.sw.SwRMXServerSocket;
import com.nextlabs.cadrmx.sw.SwRMXResult;
import com.nextlabs.cadrmx.sw.SwRMXStatus;

public class SwimRMXSession {
 	public static boolean _isSldWorksRMXRunning = false;
	private static final String _hostName = "localhost";
	private static final int _portNum = 27215;
	static SwRMXClientSocket _rmxClientSocket;
	public static int _connStat ;
	private static Boolean _saveAuxFileWhenCheckIn = null;
	private static Boolean _fileCopyMonitorEnable = Boolean.FALSE;
	private static String _rmxTempDir = null;
	
	private static final String MESSAGEDLG_TITLE = "NextLabs Rights Management Alert";
	private static final String RMX_RIGHT_SAVE = "save";
	private static final String RMX_RIGHT_SAVEAS = "saveas";
	private static final String RMX_RIGHT_EXPORT = "export";
	
	/**
	 * @return the _saveAuxFileWhenCheckIn
	 */
	public static Boolean saveAuxFileEnabled() {
		return _saveAuxFileWhenCheckIn;
	}

	/**
	 * @param setSaveAuxFileEnabled the _saveAuxFileWhenCheckIn to set
	 */
	public static void setSaveAuxFileEnabled(Boolean saveAutoFileEnabled) {
		_saveAuxFileWhenCheckIn = saveAutoFileEnabled;
	}
	
	public static Boolean isCopyMonitorEnabled() {
		return _fileCopyMonitorEnable;
	}
	
	public static void setCopyMonitorEnabled(Boolean enable) {
		_fileCopyMonitorEnable = enable;
	}
	
	static{
		
		//Start server socket in separate thread
		Thread th = new Thread(new SwRMXServerSocket()); 
        th.start();
		
        //Start the client socket
        _rmxClientSocket = new SwRMXClientSocket(_hostName, _portNum);
		_connStat = _rmxClientSocket.init();	
		if(_connStat != SwRMXStatus.SOCKET_INIT_SUCCESS.getValue()){
			//ShowMessageDialog(SwRMXStatus.SOCKET_CONNECTION_NOT_ESTABLISHED.getValue(),"");
			RMXLogHolder.error("Socket connection cannot be established." + "HostName = " + _hostName + "," +" Port Number =" + Integer.toString(_portNum) +" Failed Reasons : " +_connStat);
		}
		else{
			_isSldWorksRMXRunning = true;
			RMXLogHolder.info("Socket connection successfully established." + "HostName = " + _hostName + "," +" Port Number =" + Integer.toString(_portNum));
		}
	}
	
	public SwimRMXSession() {
		RMXLogHolder.info("SwimRMXSession Constructor");
	}
	
	public static void notifyRMXRunning(boolean running) {
		_isSldWorksRMXRunning = running;
		if (!running) {
			 _connStat = SwRMXStatus.SOCKET_CONN_RESET.getValue();
		}
	}
	
	private static SwRMXResult ConnectSocket() {
		SwRMXResult opResult = new SwRMXResult();
		if(_connStat != SwRMXStatus.SOCKET_INIT_SUCCESS.getValue()){
			RMXLogHolder.info("Previous socket connection not valid . Trying to connect again...");
			_connStat = _rmxClientSocket.init();
			if(_connStat != SwRMXStatus.SOCKET_INIT_SUCCESS.getValue()){
				RMXLogHolder.error("Attempt to re-establish socket connection failed.");
				opResult.setOperationStatus(false);
				opResult.setError(SwRMXStatus.SOCKET_CONNECTION_NOT_ESTABLISHED.getValue());	
			}
		}
		
		return opResult;
	}
	
	public static boolean initialize() {
		RMXLogHolder.info("SwimRMXSession initializing...");
			
		return true;
	}
	
	public static boolean isRMXRunning() {
		return _isSldWorksRMXRunning;
	}
	
	public static boolean nxlPerformAccessCheck(String fileName) throws Exception {
		
		boolean authorizeStatus = true;
		if(!_isSldWorksRMXRunning) {
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
			RMXLogHolder.info("Action :" +action + " Operation: " + operation);
			authorizeStatus = nxlAuthorizationCheck(action,operation,fileName);
			if(!authorizeStatus) {
				RMXLogHolder.error("User not authorized to perform " + "Action :" +action + "," +" Operation: " + operation);
			}
        }
        else {
        	RMXLogHolder.error("File doesn't exists : " +fileName);
        }
        return authorizeStatus;
	}
	
	public static boolean nxlProcessResponseFile(String fileName) throws Exception {
		boolean status = false;
		if(!_isSldWorksRMXRunning) {
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
            RMXLogHolder.info("Action: " + action);	
        }
        else {
        	RMXLogHolder.error("File doesn't exists : " +fileName);
        }
        
        if(action != null && !action.isEmpty()) {
        	  SwRMXResult responseResult;		
              //Make call to SolidWorks RMX again in order to enable it to make a map of source->destination files
                if(action.equalsIgnoreCase("new")) {
                	responseResult = sendOperationResponse(OperationType.PROCESS_SAVE_NEW.getValueString(),fileName);
                	status = responseResult.getOperationStatus();
                	if(!status)
                		RMXLogHolder.error("Error occured while processing the response xml file for action : " +action);
                }
                else if (action.equalsIgnoreCase("replace")){
                	responseResult = sendOperationResponse(OperationType.PROCESS_SAVE_REPLACE.getValueString(),fileName);
                	status = responseResult.getOperationStatus();
                	if(!status)
                		RMXLogHolder.error("Error occured while processing the response xml file for action : " +action);
                }
                else {
                	RMXLogHolder.warn("Not a supported action type. Action : " +action);
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
						RMXLogHolder.warn("Not a supported operation type. Action : " +action + " Operation : " +operation +" Setting authorizeStatus to true.");
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
			RMXLogHolder.warn("Not a supported action type. Action : " +action + " Operation : " +operation + " Setting authorizeStatus to true.");
		}
		return authorizeStatus;
	}
			
	static SwRMXResult  checkUserAuthorization(String opType,String fileName) {
		RMXLogHolder.info("OperationType :" + opType);
		SwRMXResult opResult = ConnectSocket();
		if(_connStat != SwRMXStatus.SOCKET_INIT_SUCCESS.getValue()){
			return opResult;
		}
		opResult = _rmxClientSocket.sendDataToRMXServerSocket(opType+fileName);
		if(opResult.getError() == SwRMXStatus.SOCKET_CONN_RESET.getValue()){
			RMXLogHolder.error("Socket connection reset. Possibly there is no server socket listening for the incoming client request. Please check the connection.");
			opResult.setOperationStatus(false);
			return opResult;
		}
		
		RMXLogHolder.info("Operation : " +opType + "," +" Data received from server : " +opResult.getReadData());
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
		RMXLogHolder.info("OperationType :" + opType + "," +" FileName : " + fileName);
		SwRMXResult opResult = ConnectSocket();
		if(_connStat != SwRMXStatus.SOCKET_INIT_SUCCESS.getValue()){
			return opResult;
		}
		opResult=_rmxClientSocket.sendDataToRMXServerSocket(opType+fileName);
		if(opResult.getError() == SwRMXStatus.SOCKET_CONN_RESET.getValue()){
			RMXLogHolder.error("Socket connection reset. Possibly there is no server socket listening for the incoming client request. Please check the connection.");
			opResult.setOperationStatus(false);
			return opResult;
		}
		
		RMXLogHolder.info("Operation : " +opType + "," +" Data received from server : " +opResult.getReadData());
		int errStatus = Integer.parseInt(String.valueOf(opResult.getReadData().charAt(0)));
		if(errStatus != SwUserAuthErrReasons.TC_SUCCESS.getValue()){
			RMXLogHolder.error("Error occured while processing the response xml on the server.");
			opResult.setOperationStatus(false);
		}
		else {
			opResult.setOperationStatus(true);
		}
		
		return opResult;
	}
	
	private static SwRMXResult checkRight(String fileName,  String right) {
		RMXLogHolder.info("Entering SwimRMXSession.checkRight. filename:" + fileName + ", right:" + right);
		
		SwRMXResult opResult = ConnectSocket();
		if(_connStat != SwRMXStatus.SOCKET_INIT_SUCCESS.getValue()){
			return opResult;
		}
		// Argument will be parsed by server socket, with format right:filename. e.g.: saveas|c:\test.sldprt
		opResult = _rmxClientSocket.sendDataToRMXServerSocket(right+"|" + fileName);
		if(opResult.getError() == SwRMXStatus.SOCKET_CONN_RESET.getValue()){
			RMXLogHolder.error("Socket connection reset. Possibly there is no server socket listening for the incoming client request. Please check the connection.");
			opResult.setOperationStatus(false);
			return opResult;
		}
		
		RMXLogHolder.info("Data received from server : " +opResult.getReadData());
		int errStatus = Integer.parseInt(String.valueOf(opResult.getReadData().charAt(0)));
		if(errStatus != SwUserAuthErrReasons.TC_SUCCESS.getValue()){
			//opResult.setOperationType(Integer.parseInt(opType));
			opResult.setOperationStatus(false);
			opResult.setError(SwRMXStatus.USER_NOT_AUTHORISED.getValue());
			opResult.setErrorReason(errStatus);
		}
		else {
			opResult.setOperationStatus(true);
		}
		
		RMXLogHolder.info("Exiting SwimRMXSession.checkRight. Result: " + opResult.getReadData());
		return opResult;
	}
	
	public static SwRMXResult checkSaveRight(String fileName) {
		RMXLogHolder.info("Entering SwimRMXSession.checkSaveRight. filename:" + fileName);
		boolean hasRight = true;
		
		SwRMXResult opResult = checkRight(fileName, RMX_RIGHT_SAVE);
		
		RMXLogHolder.info("Exiting SwimRMXSession.checkSaveRight. Result: " + opResult.getReadData());
		
		return opResult;
	}

	public static SwRMXResult checkSaveAsRight(String fileName) {
		RMXLogHolder.info("Entering SwimRMXSession.checkSaveAsRight. filename:" + fileName);
		boolean hasRight = true;
		
		SwRMXResult opResult = checkRight(fileName, RMX_RIGHT_SAVEAS);
		
		RMXLogHolder.info("Exiting SwimRMXSession.checkSaveAsRight. Result: " + opResult.getReadData());
		
		return opResult;
	}
	
	public static SwRMXResult checkExportRight(String fileName) {
		RMXLogHolder.info("Entering SwimRMXSession.checkExportRight. filename:" + fileName);
		boolean hasRight = true;
		
		SwRMXResult opResult = checkRight(fileName, RMX_RIGHT_EXPORT);
		
		RMXLogHolder.info("Exiting SwimRMXSession.checkExportRight. Result: " + opResult.getReadData());
		
		return opResult;
	}
	
	public static void promptAlertDialog(String errorMessage) throws Exception {
		/*JFrame frame = new JFrame("Dialog");
		// Force user exit dialogs/alerts on top of integration dialogs
		frame.setAlwaysOnTop(true);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		JOptionPane.showMessageDialog(frame,message,windowTitle, JOptionPane.ERROR_MESSAGE);*/
		
		JFrame parent = new JFrame();
        // Force user exit dialogs/alerts on top of integration dialogs
        parent.setAlwaysOnTop(true);
        parent.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        parent.setVisible(true);
        parent.setVisible(false);
        String[] button = new String[] { "Close" };
        JOptionPane.showOptionDialog(parent, errorMessage, MESSAGEDLG_TITLE, JOptionPane.DEFAULT_OPTION,
                JOptionPane.ERROR_MESSAGE, null, button, button[0]);
        parent.dispose();
	}
	
	public static String getRMXTempDir() {
		if (_rmxTempDir == null) {
			File rmxTempDir = new File(Paths.get(System.getProperty("java.io.tmpdir"), "swimrmx").toString());
			if (!rmxTempDir.exists() && !rmxTempDir.mkdirs()) {
				RMXLogHolder.error("Cannot create ipemrmx temp folder: " + rmxTempDir);
			} else {
				_rmxTempDir = rmxTempDir.getAbsolutePath();
			}
		}

		return _rmxTempDir;
	}
	
	public static SwRMXResult sendSaveAsResponse(String fileName) {
		RMXLogHolder.info("Entering SwimRMXSession.sendSaveAsResponse. filename:" + fileName);
		
		SwRMXResult opResult = ConnectSocket();
		if(_connStat != SwRMXStatus.SOCKET_INIT_SUCCESS.getValue()){
			return opResult;
		}
		// Argument will be parsed by server socket, with format right:filename. e.g.: saveas|c:\test.sldprt
		opResult = _rmxClientSocket.sendDataToRMXServerSocket("saveasresponse|" + fileName);
		if(opResult.getError() == SwRMXStatus.SOCKET_CONN_RESET.getValue()){
			RMXLogHolder.error("Socket connection reset. Possibly there is no server socket listening for the incoming client request. Please check the connection.");
			opResult.setOperationStatus(false);
			return opResult;
		}
		
		RMXLogHolder.info("Data received from server : " +opResult.getReadData());
		int errStatus = Integer.parseInt(String.valueOf(opResult.getReadData().charAt(0)));
		if(errStatus != SwUserAuthErrReasons.TC_SUCCESS.getValue()){
			//opResult.setOperationType(Integer.parseInt(opType));
			opResult.setOperationStatus(false);
			RMXLogHolder.error("Error occured while processing the response xml on the server.");
		}
		else {
			opResult.setOperationStatus(true);
		}
		
		RMXLogHolder.info("Exiting SwimRMXSession.sendSaveAsResponse. Result: " + opResult.getReadData());
		return opResult;
	}
}
