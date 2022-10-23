package com.nextlabs.swrmx.socket;
import java.net.Socket;
import java.io.StringWriter;
import java.io.PrintWriter;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.UnknownHostException;
import java.io.IOException;
import com.nextlabs.swrmx.logger.SwRMXSwimLogger;
import java.util.logging.Logger;
import com.nextlabs.swrmx.nxlutils.SwRMXResult;
import com.nextlabs.swrmx.nxlutils.SwRMXStatus;



public class SwRMXClientSocket { 
	private String hostName;
	private int portNum;
	private Socket rmxClientSocket=null;
	private PrintWriter out;
	private BufferedReader in;
	public static final SwRMXSwimLogger logger = SwRMXSwimLogger.getInstance();
	StringWriter sw = new StringWriter();
	PrintWriter pw = new PrintWriter(sw);
	
	public SwRMXClientSocket(String hostName,int portNum){
		this.hostName=hostName;
		this.portNum=portNum;
	}

	public int init(){
		SwRMXStatus status = SwRMXStatus.SOCKET_INIT_SUCCESS;
		Socket rmxClientSocket;
		try{
			rmxClientSocket = new Socket(hostName, portNum);
			if(rmxClientSocket == null){
				status = SwRMXStatus.SOCKET_CONNECT_FAIL;
				return status.getValue();
			}
		}
		catch(IllegalArgumentException e ){
			e.printStackTrace(pw);
			logger.error(sw.toString());
			status = SwRMXStatus.SOCKET_CONNECT_FAIL;
			return status.getValue();
	    }
		catch(NullPointerException e){
			e.printStackTrace(pw);
			logger.error(sw.toString());
			status = SwRMXStatus.SOCKET_CONNECT_FAIL;
			return status.getValue();
		}
		catch(UnknownHostException e){
			e.printStackTrace(pw);
			logger.error(sw.toString());
			status = SwRMXStatus.SOCKET_CONNECT_FAIL;
			return status.getValue();
		}
		catch(IOException e){
			e.printStackTrace(pw);
			logger.error(sw.toString());
			status = SwRMXStatus.SOCKET_CONNECT_FAIL;
			return status.getValue();
		}
		try{
			out = new PrintWriter(rmxClientSocket.getOutputStream(), true);
			if(out == null){
				status = SwRMXStatus.PRINTWRITER_OPEN_FAIL;
				return status.getValue();
			}
		}
		catch(IOException e){
			e.printStackTrace(pw);
			logger.error(sw.toString());
			status = SwRMXStatus.PRINTWRITER_OPEN_FAIL;
			return status.getValue();
		}
		try{
			in = new BufferedReader(new InputStreamReader(rmxClientSocket.getInputStream()));
			if(in == null){
				status = SwRMXStatus.BUFFERREADER_OPEN_FAIL;
				return status.getValue();
			}
		}
		catch(IOException e){
			e.printStackTrace(pw);
			logger.error(sw.toString());
			status = SwRMXStatus.BUFFERREADER_OPEN_FAIL;
			return status.getValue();
		}

		return status.getValue();
	}

	public SwRMXResult sendDataToRMXServerSocket(String data){
		SwRMXResult result = new SwRMXResult();
		out.println(data);
		String readData= "";
		try{
			 in.ready();
			 readData = in.readLine();	
		}
		catch(IOException e){
			e.printStackTrace(pw);
			logger.error(sw.toString());
			int status = SwRMXStatus.SOCKET_CONN_RESET.getValue();
			result.setError(status);
			result.setOperationStatus(false);
			return result;
		}
		result.setError(SwRMXStatus.SOCKET_INIT_SUCCESS.getValue());
		result.setReadData(readData);
		return result;
	}
	
	public boolean pingServer(){
		return true;
	}

   
}

