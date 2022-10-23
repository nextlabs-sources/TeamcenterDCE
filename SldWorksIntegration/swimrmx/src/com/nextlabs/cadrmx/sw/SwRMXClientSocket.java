package com.nextlabs.cadrmx.sw;

import java.net.Socket;
import java.io.StringWriter;
import java.io.PrintWriter;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.UnknownHostException;
import java.io.IOException;

import com.nextlabs.cadrmx.util.RMXLogHolder;
import com.nextlabs.cadrmx.sw.SwRMXResult;
import com.nextlabs.cadrmx.sw.SwRMXStatus;

public class SwRMXClientSocket { 
	private String _hostName;
	private int _portNum;
	private PrintWriter _out;
	private BufferedReader _in;

	StringWriter _sw = new StringWriter();
	PrintWriter _pw = new PrintWriter(_sw);
	
	public SwRMXClientSocket(String hostName,int portNum){
		this._hostName=hostName;
		this._portNum=portNum;
	}

	public int init(){
		SwRMXStatus status = SwRMXStatus.SOCKET_INIT_SUCCESS;
		Socket rmxClientSocket;
		try{
			rmxClientSocket = new Socket(_hostName, _portNum);
			if(rmxClientSocket == null){
				status = SwRMXStatus.SOCKET_CONNECT_FAIL;
				return status.getValue();
			}
		}
		catch(IllegalArgumentException e ){
			e.printStackTrace(_pw);
			RMXLogHolder.error(_sw.toString());
			status = SwRMXStatus.SOCKET_CONNECT_FAIL;
			return status.getValue();
	    }
		catch(NullPointerException e){
			e.printStackTrace(_pw);
			RMXLogHolder.error(_sw.toString());
			status = SwRMXStatus.SOCKET_CONNECT_FAIL;
			return status.getValue();
		}
		catch(UnknownHostException e){
			e.printStackTrace(_pw);
			RMXLogHolder.error(_sw.toString());
			status = SwRMXStatus.SOCKET_CONNECT_FAIL;
			return status.getValue();
		}
		catch(IOException e){
			e.printStackTrace(_pw);
			RMXLogHolder.error(_sw.toString());
			status = SwRMXStatus.SOCKET_CONNECT_FAIL;
			return status.getValue();
		}
		try{
			_out = new PrintWriter(rmxClientSocket.getOutputStream(), true);
			if(_out == null){
				status = SwRMXStatus.PRINTWRITER_OPEN_FAIL;
				return status.getValue();
			}
		}
		catch(IOException e){
			e.printStackTrace(_pw);
			RMXLogHolder.error(_sw.toString());
			status = SwRMXStatus.PRINTWRITER_OPEN_FAIL;
			return status.getValue();
		}
		try{
			_in = new BufferedReader(new InputStreamReader(rmxClientSocket.getInputStream()));
			if(_in == null){
				status = SwRMXStatus.BUFFERREADER_OPEN_FAIL;
				return status.getValue();
			}
		}
		catch(IOException e){
			e.printStackTrace(_pw);
			RMXLogHolder.error(_sw.toString());
			status = SwRMXStatus.BUFFERREADER_OPEN_FAIL;
			return status.getValue();
		}

		return status.getValue();
	}

	public SwRMXResult sendDataToRMXServerSocket(String data){
		RMXLogHolder.info("sendDataToRMXServerSocket: " + data);
		SwRMXResult result = new SwRMXResult();
		_out.println(data);
		_out.flush();
		String readData= "";
		try{
			 _in.ready();
			 readData = _in.readLine();	
		}
		catch(IOException e){
			e.printStackTrace(_pw);
			RMXLogHolder.error(_sw.toString());
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

