package com.nextlabs.swrmx.nxlutils;
import com.nextlabs.swrmx.nxlutils.SwRMXStatus;
import com.nextlabs.swrmx.nxlutils.OperationType;
import javax.swing.JFrame;
import javax.swing.JOptionPane;


public class SwRMXResult{

	public SwRMXResult(){
	
	}
	
	private int error;
	private int errorReason;
	private String readData;
	private int operationType;
	private boolean opStatus;
	private static final String windowTitle = "NextLabs Rights Management eXtension";
	
	public void setError(int err){
		error = err;
	}
	
	public void setErrorReason(int errReason) {
		errorReason = errReason;
	}
	
	public void setReadData(String rd){
	    readData = rd;
	}
	
	
	public String getReadData(){
		return readData;
	}
	
	public int getError(){
		return error;
	}
	
	public int getErrorReason() {
		return errorReason;
	}
	
	public void setOperationType(int opType){
		operationType = opType;
	}
	
	public int getOperationType(){
		return operationType;
	}
	
	public void setOperationStatus(boolean status){
		opStatus = status;
	
	}
	
	public boolean getOperationStatus(){
		return opStatus;
	}
	
	public void showErrorMessage(){
		
		int errVal = getError();	
		if(errVal == SwRMXStatus.USER_NOT_AUTHORISED.getValue()){
			String errStr = getReadData();
			String errSubStr = getReadData().substring(1,errStr.length());
			String[] tokens = errSubStr.split("\\|");
			String msg = "";
			String delim = "";
			for(String token: tokens){
				msg = msg + delim + token;
				delim = "\r\n";
			}
			int errReason = getErrorReason();
			msg = getErrMsgString(errVal,errReason) + msg;
			showMessageDialog(errVal,msg);		
		}
		else{
			String msg = getErrMsgString(errVal,0) ;
			showMessageDialog(errVal,msg);
			
		}	
	}
	
	public String getErrMsgString(int errVal,int errReason){
        String msg = "";	
		if(errVal == SwRMXStatus.SOCKET_CONNECTION_NOT_ESTABLISHED.getValue()){
			msg = "Swim failed to establish socket connection with the server socket.";
		}
		else if(errVal == SwRMXStatus.SOCKET_CONN_RESET.getValue()){
			msg = "Socket connection was reset.";
		}
		
		else if(errVal == SwRMXStatus.USER_NOT_AUTHORISED.getValue()){
			if(errReason == SwUserAuthErrReasons.TC_AUTHFAIL_RIGHTEDIT.getValue()){
				msg = "Operation Denied.\r\n You do not have 'EDIT' permission to perform this action on the following file(s).Please deselect them and try again.\r\n" ;
				  
			}
			else if(errReason == SwUserAuthErrReasons.TC_AUTHFAIL_RIGHTDOWNLOAD.getValue()){
				msg = "Operation Denied.\r\n You do not have 'SAVEAS OR DOWNLOAD' permission to perform this action on the following file(s).Please deselect them and try again.\r\n" ;	
			}
			else if(errReason == SwUserAuthErrReasons.TC_AUTHFAIL_DOWNLOAD_NORIGHTEDIT.getValue()) {
				msg = "Operation Denied.\r\n You are not allowed to perform this action because the following files(s) with no 'EDIT' permission were modified.Please deselect them and try again.\r\n";
			}
			else if(errReason == SwUserAuthErrReasons.TC_FILE_PARSE_ERROR.getValue()) {
				msg = "Error occured while authorizing user action due to parsing the input xml file.\r\n Please check nxlswim.log or SldWorksRMX.log for more information.";
			}
			else if(errReason == SwUserAuthErrReasons.TC_AUTHFAIL_EDIT_NOJTSAVERIGHT.getValue()) {
				msg = "Operation Denied.\r\n You do not have 'SAVEAS OR DOWNLOAD' permission to generate JT files associated with the following file(s).Please deselect them and try again.\r\n";
			}
			else if(errReason == SwUserAuthErrReasons.TC_AUTHFAIL_REVISE_NOEDITRIGHT.getValue()) {
				msg = "Operation Denied.\r\n You do not have 'EDIT' permission to create new revision for the following file(s).Please deselect then and try again.\r\n";
			}
			else if(errReason == SwUserAuthErrReasons.TC_AUTHFAIL_REVISE_NOSAVEASRIGHT.getValue()) {
				msg = "Operation Denied.\r\n You do not have 'SAVEAS OR DOWNLOAD' permission to create new revision for the following file(s).Please deselect then and try again.\r\n";
			}
		}
		return msg;
	
	}
	
	
	
	public void showMessageDialog(int reasons,String message){
		JFrame frame = new JFrame("Dialog");
		frame.setAlwaysOnTop(true);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		JOptionPane.showMessageDialog(frame,message,windowTitle, JOptionPane.ERROR_MESSAGE);
	}


}